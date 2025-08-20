//
// Created by paull on 2023-01-13.
//

#include "VulkanBufferSuballocator.hpp"

#include <iostream>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace pEngine::girEngine::backend::vulkan {
    void
    VulkanBufferSuballocator::suballocateBuffers(
        const std::vector<gir::BufferIR> &buffersToSuballocate) {
        if (buffersToSuballocate.empty()) {
            return;
        }

        const size_t newBufferSize = computeNewBufferSize(buffersToSuballocate);

        createVulkanBufferIfNonexistent(newBufferSize);

        defragmentIfNeeded();

        suballocateBufferList(buffersToSuballocate);
    }

    void VulkanBufferSuballocator::validateGreatestMemoryBlockOffset(size_t greatestUsedMemoryBlockOffset) const {
        for (auto &usedBlock: suballocatedBlocks) {
            if (usedBlock.offset > greatestUsedMemoryBlockOffset) {
                // this shouldn't ever happen since they're being sorted in ascending order (due to std::set)
                greatestUsedMemoryBlockOffset = usedBlock.offset;
                // TODO - probably have this log an error or maybe even throw an exception
            }
        }
    }

    void VulkanBufferSuballocator::defragmentIfNeeded() {
        computeCurrentAmountOfFragmentation();
        if (currentDefragmentationAmount > defragmentationThreshold) {
            defragmentBuffer();
        }
    }

    void VulkanBufferSuballocator::defragmentBuffer() {
        // alright this is a fairly hefty function I believe!
        // the idea will be to "left-shift" everything so that we end up with a singular free block
        // containing all addresses greater than (offset + size) for the last (in ascending order) used block;
        // I think that means we can just get rid of all the free blocks in one go tho;

        // as long as we have the used blocks ordered ascending by offset, then we should be able to just
        // basically:
        // 1. wipe the freelist (actually this can probably done as part of step 4)
        // 2. create a temporary vector for storing the new used blocks
        // 3. fill the vector by going through the current used blocks in order and copying them, updating the offset
        // by adding each used block's size as we copy them in
        // 4. create a free block for all the remaining buffer space
        // 5. assign the current used blocks to be the new vector of used blocks we just created

        // then from that point on, we should have the same buffer but defragmented (not sure about synchronization yet)
        defragmentUsedBlocksList();
        defragmentFreeBlockList();
    }

    void VulkanBufferSuballocator::defragmentUsedBlocksList() {
        boost::container::flat_set<MemoryBlock, std::less<> > newSuballocatedBlockList = {};

        size_t newOffset = 0u;
        for (auto &currentUsedBlock: suballocatedBlocks) {
            newSuballocatedBlockList.insert(
                MemoryBlock(newOffset, currentUsedBlock.size, currentUsedBlock.suballocationUid));
            newOffset += currentUsedBlock.size;
        }
        if (newSuballocatedBlockList.size() != suballocatedBlocks.size()) {
            throw std::runtime_error(
                "Error in VulkanBufferSuballocator::defragmentBuffer() - not all original suballocations were recreated during defragmentation!");
        }
        suballocatedBlocks = newSuballocatedBlockList;
    }

    void VulkanBufferSuballocator::defragmentFreeBlockList() {
        freeBlocks.clear();
        freeBlocks.insert(MemoryBlock(
            suballocatedBlocks.end()->offset,
            bufferSize - suballocatedBlocks.size(),
            suballocatedBlocks.end()->suballocationUid
        ));
    }

    /**
     * I'll have to redo this design later, I have a feeling it's disgustingly inefficient
     */
    void VulkanBufferSuballocator::computeCurrentAmountOfFragmentation() {
        size_t fragmentedMemoryBytes = 0;

        if (suballocations.empty()) {
            return;
        }

        // hmm... need to figure out an approach for doing this
        // i'm thinking we could just find the offset with the greatest memory address, then (sort?) and loop
        // over all the free memory blocks and add their size to the count if they're below that offset?
        const size_t greatestUsedMemoryBlockOffset = (suballocatedBlocks.end()--)->offset; // TODO fix this ugly shite
        validateGreatestMemoryBlockOffset(greatestUsedMemoryBlockOffset); // TODO - make this disable-able

        for (const auto &freeBlock: freeBlocks) {
            if (freeBlock.offset < greatestUsedMemoryBlockOffset) {
                fragmentedMemoryBytes += freeBlock.size;
            }
        }

        currentDefragmentationAmount = static_cast<double>(fragmentedMemoryBytes) / static_cast<double>(bufferSize);
    }

    /**
     * NOTE - this doesn't seem to be safe at all if the suballocator
     * has already got some suballocations;
     * I'll have to adapt it in the future
     */
    void VulkanBufferSuballocator::suballocateBufferList(const std::vector<gir::BufferIR> &buffers) {
        size_t currentSuballocationOffset = 0;
        for (const auto &bufferToSuballocate: buffers) {
            uint32_t padding = 0;
            unsigned bufferAllocationSize = bufferToSuballocate.maximumBufferSizeInBytes.get_value_or(
                bufferToSuballocate.rawDataContainer.getRawDataSizeInBytes());
            if (uint32_t spillover = bufferAllocationSize % minimumBufferAlignment; spillover != 0) {
                padding = minimumBufferAlignment - spillover;
            }

            suballocateBuffer(bufferToSuballocate, currentSuballocationOffset);
            currentSuballocationOffset += bufferAllocationSize + padding;
        }
    }

    void VulkanBufferSuballocator::createVulkanBufferIfNonexistent(
        const size_t newBufferSize) {
        if (buffer == nullptr) {
            bufferSize = newBufferSize;
            createVulkanBuffer();
        }
    }

    size_t VulkanBufferSuballocator::computeNewBufferSize(
        const std::vector<gir::BufferIR> &buffersToSuballocate) const {
        size_t newBufferSize = bufferSize;
        for (const auto &bufferToSuballocate: buffersToSuballocate) {
            // OKAY -> new error caused by changes to model code: vertex buffer data is not being passed into girs
            // above error is fixed, now we have vertex data, but there's some weird thing going on where both meshes are ending up with
            // the same data (maybe to do with how we're mapping our suballocations to vertex buffers at draw time)

            // similarly, here we need to consider the alignment of the buffer
            uint32_t padding = 0;
            size_t rawDataSizeInBytes = bufferToSuballocate.maximumBufferSizeInBytes.get_value_or(
                bufferToSuballocate.rawDataContainer.getRawDataSizeInBytes());
            if (uint32_t spillover = rawDataSizeInBytes % minimumBufferAlignment; spillover != 0) {
                padding = minimumBufferAlignment - spillover;
            }

            if (rawDataSizeInBytes) {
                newBufferSize += rawDataSizeInBytes + padding;
            } else {
                if (!bufferToSuballocate.maximumBufferSizeInBytes.has_value()) {
                    // TODO - log that this happened!
                    return 0;
                }
                newBufferSize += bufferToSuballocate.maximumBufferSizeInBytes.value() + padding;
            }
        }
        return newBufferSize;
    }

    void VulkanBufferSuballocator::createVulkanBuffer() {
        // DEBUG: no clue why the allocator is ending up null here
        buffer = std::make_shared<VulkanBuffer>(
            VulkanBuffer::CreationInput
            {
                allocator,
                bufferSize,
                bufferUsageFlags,
                queueFamilyIndices,
                VMA_MEMORY_USAGE_AUTO,
                true,
                true
            }
        );
        allocation = buffer->getBufferAllocation();
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::suballocateBuffer(const gir::BufferIR &bufferToSuballocate,
                                                     size_t suballocationOffset) {
        copyBufferDataToMappedVulkanBuffer(bufferToSuballocate, suballocationOffset);
    }


    /**
     * Alright - this is actually a bit flawed because I did not know that you can only map
     * HOST_VISIBLE memory, and not all GPUs support it / it's also not the most efficient thing ever
     *
     * This is a lil awkward because the alternative is a buffer-buffer copy via some kind of staging buffer,
     * so it seems likely that I'll probably have to write a version of this function that
     * takes in a queue and fence and what not;
     *
     * Although tbh, it might be better to just "skip" initialization stuff and let the data be copied in
     * at run-time (until a future update where we'll have some kind of CopyAggregator abstraction where you
     * register things to be updated at the beginning of the frame with it and it submits them all at once
     *
     * @param bufferToSuballocate
     * @param suballocationOffset
     */
    void
    VulkanBufferSuballocator::copyBufferDataToMappedVulkanBuffer(
        const gir::BufferIR &bufferToSuballocate,
        const size_t suballocationOffset) {
        // FOR NOW -> non-host-visible memory initialization is skipped, to be added (probably) in the next big update
        bool bufferIsHostVisible = false;
        if (bufferIsHostVisible) {
            void *mappedBuffer = nullptr;
            if (!buffer->mapVulkanBufferMemoryToPointer(&mappedBuffer)) {
                // TODO - log!
            }

            if (mappedBuffer == nullptr) {
                // TODO - log!
                throw std::runtime_error("Failed to subllocate buffer!");
            }

            // copy the raw data from the buffer being suballocated at the given offset
            std::memcpy(
                (unsigned char *) mappedBuffer + suballocationOffset,
                // TODO - add sanity checking so you can't inject code / buffer overflow
                bufferToSuballocate.rawDataContainer.getRawDataPointer(),
                bufferToSuballocate.rawDataContainer.getRawDataSizeInBytes()
            );

            // unmap the memory
            buffer->unmapVulkanBufferMemory();

            // note; i'm fairly certain VMA allows you to map a single buffer from multiple threads, so ideally
            // we won't need synchronization/duplication across threads
        }

        // add vulkan buffer suballocation
        suballocations.insert(
            {
                bufferToSuballocate.uid,
                VulkanBufferSuballocation{
                    bufferToSuballocate.uid,
                    bufferToSuballocate.maximumBufferSizeInBytes.get_value_or(
                        bufferToSuballocate.rawDataContainer.getRawDataSizeInBytes()),
                    suballocationOffset,
                    bufferToSuballocate.bindingIndex,
                    boost::optional<std::vector<uint8_t> >(
                        bufferToSuballocate.rawDataContainer.getRawDataAsVector<uint8_t>())
                }
            }
        );

        // add suballocation memory block
        suballocatedBlocks.insert(MemoryBlock(
                suballocationOffset,
                bufferToSuballocate.maximumBufferSizeInBytes.get_value_or(
                    bufferToSuballocate.rawDataContainer.getRawDataSizeInBytes()),
                bufferToSuballocate.uid)
        );
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::freeSuballocationsByUid(
        const std::vector<util::UniqueIdentifier> &suballocatedBufferUids) {
        if (suballocatedBlocks.empty()) {
            // TODO - probably log or throw here
            return;
        }

        checkThatRequestedBufferUidsArePresent(suballocatedBufferUids);

        for (const auto &suballocatedBufferUid: suballocatedBufferUids) {
            freeSuballocatedBuffer(suballocatedBufferUid);
        }
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::checkThatRequestedBufferUidsArePresent(
        const std::vector<util::UniqueIdentifier> &suballocatedBufferUids) {
        for (auto &suballocatedBufferUid: suballocatedBufferUids) {
            auto findResult = std::find_if(suballocatedBlocks.begin(), suballocatedBlocks.end(),
                                           [&](const MemoryBlock &block) {
                                               return block.suballocationUid == suballocatedBufferUid;
                                           });
            if (findResult == suballocatedBlocks.end()) {
                // TODO - log!
            }
        }
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::freeSuballocatedBuffer(const util::UniqueIdentifier &suballocatedBufferUid) {
        if (suballocatedBlocks.empty()) {
            return;
        }

        const auto suballocatedBufferIterator = std::find_if(suballocatedBlocks.begin(), suballocatedBlocks.end(),
                                                             [&](const MemoryBlock &block) {
                                                                 return block.suballocationUid == suballocatedBufferUid;
                                                             });

        suballocatedBlocks.erase(suballocatedBufferIterator);
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::freeAllBufferSuballocations() {
        if (suballocatedBlocks.empty()) {
            return;
        }

        std::vector<util::UniqueIdentifier> usedBlockUids = {};
        for (const auto &suballocation: suballocatedBlocks) {
            usedBlockUids.push_back(suballocation.suballocationUid);
        }

        freeSuballocationsByUid(usedBlockUids);
    }

    // TODO -> re-evaluate this
    void VulkanBufferSuballocator::suballocateBuffer(const gir::BufferIR &bufferToSuballocate) {
        const std::vector temporaryBufferVector = {bufferToSuballocate};
        suballocateBuffers(temporaryBufferVector);
    }
} // PGraphics
