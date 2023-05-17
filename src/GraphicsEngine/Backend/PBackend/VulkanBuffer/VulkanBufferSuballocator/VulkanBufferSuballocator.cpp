//
// Created by paull on 2023-01-13.
//

#include "VulkanBufferSuballocator.hpp"

namespace PGraphics {

    void
    VulkanBufferSuballocator::suballocateBuffers(const std::vector<std::shared_ptr<Buffer>> &buffersToSuballocate) {
        unsigned long newBufferSize = computeNewBufferSize(buffersToSuballocate);
        validateNewBufferSize(newBufferSize);

        createVulkanBufferIfItDoesNotAlreadyExist(newBufferSize);

        defragmentIfNeeded();

        suballocateSceneBuffers(buffersToSuballocate);
    }

    void VulkanBufferSuballocator::validateGreatestMemoryBlockOffset(unsigned long greatestUsedMemoryBlockOffset) {
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
        // 3. fill the vector by going through the current used blocks in order and copying them, resetting the offset to
        // be whatever it should be at that point
        // 4. create a free block for all the remaining buffer space
        // 5. assign the current used blocks to be the new vector of used blocks we just created

        // then from that point on, we should have the same buffer but defragmented (not sure about synchronization yet)
        defragmentUsedBlocksList();
        defragmentFreeBlockList();
    }

    void VulkanBufferSuballocator::defragmentUsedBlocksList() {
        std::set<MemoryBlock> newSuballocatedBlockList = {};

        unsigned long newOffset = 0;
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

        unsigned long newFreeBlockOffset = suballocatedBlocks.end()->offset;
        unsigned long newFreeBlockSize = bufferSize - suballocatedBlocks.size();
        UniqueIdentifier newFreeBlockSuballocationUid = suballocatedBlocks.end()->suballocationUid;

        freeBlocks.insert(MemoryBlock(newFreeBlockOffset, newFreeBlockSize, newFreeBlockSuballocationUid));
    }

    void VulkanBufferSuballocator::computeCurrentAmountOfFragmentation() {
        unsigned long fragmentedMemoryBytes = 0;

        // hmm... need to figure out an approach for doing this
        // i'm thinking we could just find the offset with the greatest memory address, then (sort?) and loop
        // over all the free memory blocks and add their size to the count if they're below that offset?
        unsigned long greatestUsedMemoryBlockOffset = suballocatedBlocks.end()->offset;
        validateGreatestMemoryBlockOffset(greatestUsedMemoryBlockOffset);// TODO - make this disable-able

        for (auto &freeBlock: freeBlocks) {
            if (freeBlock.offset < greatestUsedMemoryBlockOffset) {
                fragmentedMemoryBytes += freeBlock.size;
            }
        }

        currentDefragmentationAmount = (double) fragmentedMemoryBytes / (double) bufferSize;
    }

    void
    VulkanBufferSuballocator::suballocateSceneBuffers(
            const std::vector<std::shared_ptr<Buffer>> &buffers) {
        unsigned long currentSuballocationOffset = 0;
        for (const auto &bufferToSuballocate: buffers) {
            // TODO - rewrite this
            suballocateBuffer(bufferToSuballocate, currentSuballocationOffset);
            currentSuballocationOffset += bufferToSuballocate->getSizeInBytes();
        }
    }

    void VulkanBufferSuballocator::validateNewBufferSize(unsigned long long int newBufferSize) const {
        if (newBufferSize > maximumBufferSize) {
            // for now, we'll just fail when the buffer goes over the user-defined limit;
            // TODO - add functionality where we can have the suballocator recreate a larger VulkanBuffer
            throw std::runtime_error(
                    "Error in VulkanBufferSuballocator::suballocateBuffers: buffer would be larger than user-defined maximum!");
        }
    }

    void VulkanBufferSuballocator::createVulkanBufferIfItDoesNotAlreadyExist(unsigned long long int newBufferSize) {
        if (buffer == nullptr) {
            bufferSize = newBufferSize;
            createVulkanBuffer();
        }
    }

    unsigned long long int
    VulkanBufferSuballocator::computeNewBufferSize(
            const std::vector<std::shared_ptr<Buffer>> &buffersToSuballocate) const {
        unsigned long long newBufferSize = bufferSize;
        for (const auto &bufferToSuballocate: buffersToSuballocate) {
            newBufferSize += bufferToSuballocate->getSizeInBytes();
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

    void VulkanBufferSuballocator::suballocateBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                                                     unsigned long long suballocationOffset) {

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

        createNewSuballocationForBuffer(bufferToSuballocate, suballocationOffset);
        copyBufferDataToMappedVulkanBuffer(bufferToSuballocate, suballocationOffset);
    }

    void VulkanBufferSuballocator::createNewSuballocationForBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                                                                   unsigned long long int suballocationOffset) {
        // TODO - suballocate differently depending on the type of buffer (ie the usage of it)

        VulkanBufferSuballocation::CreationInput suballocationCreationInput = {
                bufferToSuballocate->getUniqueIdentifier(),
                bufferToSuballocate->getSizeInBytes(),
                {}, // TODO - replace this
                suballocationOffset
        };

        const std::shared_ptr<VulkanBufferSuballocation> &newSuballocation = std::make_shared<VulkanBufferSuballocation>(
                suballocationCreationInput);
        suballocations.push_back(newSuballocation);
        suballocationUniqueIds[bufferToSuballocate->getUniqueIdentifier()] = suballocations.size() - 1;
    }

    void
    VulkanBufferSuballocator::copyBufferDataToMappedVulkanBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                                                                 unsigned long long int suballocationOffset) {
        // TODO - re-evaluate how this is done

        /**
         * This is a tricky issue! I'll try and write the problem out here and maybe that'll help me come up with something.
         *
         * The idea here is that we've got:
         * 1. a particular buffer suballocator which corresponds to a possible buffer usage
         * 2. a Scene::Buffer which also corresponds to that same buffer usage as the suballocator
         * 3. an offset within the suballocator's underlying buffer (which contains the individual Scene::Buffers as suballocations) where the Scene::Buffer data will be copied
         *
         * We want this to happen, in order to do a (safer) std::copy (as opposed to memcpy which I was using before):
         * 1. We get pointers to the start and end of the Scene::Buffer's data, which (NOW THAT WE'RE NOT USING
         * STD::ANY AND INSTEAD ARE USING POLYMORPHISM)
         * 2.
         */

        void *mappedBuffer = buffer->mapVulkanBufferMemory();


        buffer->unmapVulkanBufferMemory();
    }

    void VulkanBufferSuballocator::freeSuballocationsByUid(
            const std::vector<PUtilities::UniqueIdentifier> &suballocatedBufferUids) {
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
            const std::vector<PUtilities::UniqueIdentifier> &suballocatedBufferUids) {
        for (auto &suballocatedBufferUid: suballocatedBufferUids) {
            auto findResult = std::find_if(suballocatedBlocks.begin(), suballocatedBlocks.end(),
                                           [&](const MemoryBlock &block) {
                                               return block.suballocationUid == suballocatedBufferUid;
                                           });
            if (findResult == suballocatedBlocks.end()) {
                throw std::runtime_error(
                        "Error in VulkanBufferSuballocator::freeSuballocationsByUid() - caller requested to free a buffer that isn't allocated!");
            }
        }
    }

    void VulkanBufferSuballocator::freeSuballocatedBuffer(const PUtilities::UniqueIdentifier &suballocatedBufferUid) {
        if (suballocatedBlocks.empty()) {
            return;
        }

        auto suballocatedBufferIterator = std::find_if(suballocatedBlocks.begin(), suballocatedBlocks.end(),
                                                       [&](const MemoryBlock &block) {
                                                           return block.suballocationUid == suballocatedBufferUid;
                                                       });

        suballocatedBlocks.erase(suballocatedBufferIterator);
    }

    void VulkanBufferSuballocator::freeAllBuffers() {
        if (suballocatedBlocks.empty()) {
            return;
        }

        std::vector<PUtilities::UniqueIdentifier> usedBlockUids = {};
        for (const auto &suballocation: suballocatedBlocks) {
            usedBlockUids.push_back(suballocation.suballocationUid);
        }

        freeSuballocationsByUid(usedBlockUids);
    }

    void VulkanBufferSuballocator::suballocateBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate) {
        std::vector<std::shared_ptr<Buffer>> temporaryBufferVector = {bufferToSuballocate};
        suballocateBuffers(temporaryBufferVector);
    }

} // PGraphics