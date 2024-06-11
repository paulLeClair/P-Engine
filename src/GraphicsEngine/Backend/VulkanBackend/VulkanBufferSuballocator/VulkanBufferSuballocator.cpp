//
// Created by paull on 2023-01-13.
//

#include "VulkanBufferSuballocator.hpp"

namespace pEngine::girEngine::backend::vulkan {
    void
    VulkanBufferSuballocator::suballocateBuffers(
            const std::vector<std::shared_ptr<gir::BufferIR> > &buffersToSuballocate) {
        if (buffersToSuballocate.empty()) {
            return;
        }

        const size_t newBufferSize = computeNewBufferSize(buffersToSuballocate);

        createVulkanBufferIfItDoesNotAlreadyExist(newBufferSize);

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

    void
    VulkanBufferSuballocator::suballocateBufferList(
            const std::vector<std::shared_ptr<gir::BufferIR> > &buffers) {
        size_t currentSuballocationOffset = 0;
        for (const auto &bufferToSuballocate: buffers) {
            // TODO - rewrite this
            suballocateBuffer(bufferToSuballocate, currentSuballocationOffset);
            currentSuballocationOffset += bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes();
        }
    }

    void VulkanBufferSuballocator::createVulkanBufferIfItDoesNotAlreadyExist(
            const size_t newBufferSize) {
        if (buffer == nullptr) {
            bufferSize = newBufferSize;
            createVulkanBuffer();
        }
    }

    size_t VulkanBufferSuballocator::computeNewBufferSize(
            const std::vector<std::shared_ptr<gir::BufferIR> > &buffersToSuballocate) const {
        size_t newBufferSize = bufferSize;
        for (const auto &bufferToSuballocate: buffersToSuballocate) {
            newBufferSize += bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes();
        }
        return newBufferSize;
    }

    void VulkanBufferSuballocator::createVulkanBuffer() {
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
    }

    void VulkanBufferSuballocator::suballocateBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
                                                     size_t suballocationOffset) {
        /**
         * I really need to sort out the connection between Renderables and their Vertices/Indices constructs to the
         * actual VertexBuffer and IndexBuffer... they almost seem like duplicates of each other. Maybe I can get away
         * with trimming away one of them?
         *
         * Ie maybe we can only supply vertex and index buffers through Vertices and Indices, and we can have only
         * non-geometry uses for the regular Scene::Buffers (or maybe we instead move all the current Vertices&Indices stuff
         * into the VertexBuffer and IndexBuffer classes we made, and Renderables have handles to VertexBuffers and IndexBuffers instead
         *
         * I kind of prefer the latter - then we can transfer all the *Vertices and *Indices subclasses out of that `common` folder
         * and keep Renderables higher-level
         */
        copyBufferDataToMappedVulkanBuffer(bufferToSuballocate, suballocationOffset);
    }

    void
    VulkanBufferSuballocator::createNewSuballocationForBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
                                                              size_t suballocationOffset) {
        // TODO - suballocate differently depending on the type of buffer (ie the usage of it)

        VulkanBufferSuballocation::CreationInput suballocationCreationInput = {
                bufferToSuballocate->getUid(),
                bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes(),
                {}, // TODO - replace this
                suballocationOffset
        };

        const std::shared_ptr<VulkanBufferSuballocation> &newSuballocation = std::make_shared<
                VulkanBufferSuballocation>(
                suballocationCreationInput);
        suballocations.push_back(newSuballocation);
        suballocationUniqueIds[bufferToSuballocate->getUid()] = suballocations.size() - 1;
    }

    void
    VulkanBufferSuballocator::copyBufferDataToMappedVulkanBuffer(
            const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
            const size_t suballocationOffset) {
        // obtain pointer into the buffer that we suballocate in this class

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
                bufferToSuballocate->getRawDataContainer().getRawDataPointer(),
                bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes()
        );

        // unmap the memory
        buffer->unmapVulkanBufferMemory();

        // note; i'm fairly certain VMA allows you to map a single buffer from multiple threads, so ideally
        // we won't need synchronization/duplication across threads

        // add vulkan buffer suballocation
        suballocations.push_back(
                std::make_shared<VulkanBufferSuballocation>(VulkanBufferSuballocation::CreationInput{
                        bufferToSuballocate->getUid(),
                        bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes(),
                        bufferToSuballocate->getRawDataContainer().getRawDataAsVectorOfBytes(),
                        suballocationOffset
                })
        );

        // add suballocation memory block
        suballocatedBlocks.insert(MemoryBlock(
                suballocationOffset,
                bufferToSuballocate->getRawDataContainer().getRawDataSizeInBytes(),
                bufferToSuballocate->getUid())
        );
    }

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

    void VulkanBufferSuballocator::suballocateBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate) {
        const std::vector temporaryBufferVector = {bufferToSuballocate};
        suballocateBuffers(temporaryBufferVector);
    }
} // PGraphics
