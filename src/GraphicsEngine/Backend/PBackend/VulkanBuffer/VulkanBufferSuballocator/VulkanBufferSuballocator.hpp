//
// Created by paull on 2023-01-13.
//

#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>
#include <set>
#include "../../../../Scene/SceneResources/Buffer/Buffer.hpp"
#include "../VulkanBuffer.hpp"
#include "VulkanBufferSuballocation/VulkanBufferSuballocation.hpp"

namespace PGraphics {

    /**
     * Idea here: since we can't just amalgamate everything into a single buffer, we want to amalgamate
     * into buffers based on VkBufferUsage - this way buffers that are used the same way are suballocated
     * into one big buffer as is recommended.
     *
     * I'm not entirely sure how to handle dynamic geometry - maybe a simple dynamic array approach where
     * we double the size of the buffer and copy the old data into the new one whenever we want to add
     * a buffer that is too large to fit inside the current buffer.
     *
     * As far as deletion of buffers, from what I've seen it's apparently good to just keep a list of
     * free and used blocks - VMA doesn't support suballocation automatically so I'm gonna have to do it manually.
     *
     * I guess that means that I could go about this in a few ways - going for a super-simple dynamic array style allocator
     * where it just doubles and copies everything over to the new buffer when an allocation is too big might be nice (however that will lead to
     * lots of fragmentation if there are lots of freed buffers)
     *
     * If I have to worry about defragmentation anyway, it might make sense to just do a traditional allocator
     * where it keeps track of free and occupied memory, plus the information about the individual allocations that make
     * up the occupied memory. Then we just allocate new buffers on demand by looking for free memory, mapping&copying,
     * and then boom you should have yourself something that can be attached to a descriptor set and bound to a command
     * buffer!
     *
     * Buffers would also be able to be freed, and then you would just either amalgmate that freed memory with neighboring
     * free memory, or you set it as free so that it might be used by something else. Upon freeing the buffer,
     * we would also recompute the overall level of fragmentation, which could be defined as "amount of free memory that is fragmented"
     * divided by "amount of free memory that is not fragmented" - if the proportion gets beyond a user-specified level
     * it will defragment, probably by using a staging buffer to build the new contiguous buffer that we want
     */
    class VulkanBufferSuballocator {
    public:
        struct CreationInput {
            VmaAllocator allocator;

            VkBufferUsageFlags bufferUsageFlags;

            const std::vector<std::shared_ptr<Buffer>> &initialBuffers;

            /**
             * This is the maximum percentage of fragmentation that is allowed before the suballocator will initiate
             * defragmentation, where fragmentation is just the number of fragmented empty bytes divided by the total
             * of number bytes in the buffer
             */
            float defragmentationThreshold = 0.3;

            long long maximumBufferSize;

            std::vector<unsigned int> queueFamilyIndices;
        };

        explicit VulkanBufferSuballocator(const CreationInput &creationInput) :
                bufferUsageFlags(creationInput.bufferUsageFlags),
                defragmentationThreshold(creationInput.defragmentationThreshold),
                maximumBufferSize(creationInput.maximumBufferSize), allocator(creationInput.allocator), bufferSize(0),
                currentDefragmentationAmount(0) {
            suballocateBuffers(creationInput.initialBuffers);
        }

        ~VulkanBufferSuballocator() = default;

        void suballocateBuffers(const std::vector<std::shared_ptr<Buffer>> &buffersToSuballocate);

        void suballocateBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate);

        void freeSuballocatedBuffer(const PUtilities::UniqueIdentifier &suballocatedBufferUid);

        void freeSuballocationsByUid(const std::vector<PUtilities::UniqueIdentifier> &suballocatedBufferUids);

        void freeAllBuffers();

    private:
        // TODO - think about how to synchronize these across render threads

        VmaAllocator allocator;

        std::vector<unsigned int> queueFamilyIndices;

        /**
         * This is the buffer that the VulkanBufferSuballocations are stored in; it will be dynamically
         * resized when it grows beyond its current capacity, and will automatically defragment once
         * it detects that the current setup is
         */
        std::shared_ptr<VulkanBuffer> buffer;

        std::vector<std::shared_ptr<VulkanBufferSuballocation>> suballocations;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> suballocationUniqueIds;

        const VkBufferUsageFlags bufferUsageFlags;

        const float defragmentationThreshold;

        unsigned long long bufferSize;

        const long long maximumBufferSize;

        // TODO - break off fragmentation control into its own class
        struct MemoryBlock {
            unsigned long long offset;
            unsigned long long size;

            UniqueIdentifier suballocationUid;

            bool operator<(const MemoryBlock &other) const {
                return offset < other.offset;
            }

            bool operator<=(const MemoryBlock &other) const {
                return offset == other.offset || *this < other;
            }

            MemoryBlock(unsigned long long offset, unsigned long long size, const UniqueIdentifier &suballocationUid) :
                    offset(offset),
                    size(size),
                    suballocationUid(suballocationUid) {}
        };

        std::set<MemoryBlock> freeBlocks;
        std::set<MemoryBlock> suballocatedBlocks;

        double currentDefragmentationAmount;
        std::set<MemoryBlock> fragments;

        void defragmentBuffer();

        void createVulkanBuffer();

        void suballocateBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                               unsigned long long suballocationOffset);

        [[nodiscard]] unsigned long long int
        computeNewBufferSize(const std::vector<std::shared_ptr<Buffer>> &buffersToSuballocate) const;

        void copyBufferDataToMappedVulkanBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                                                unsigned long long int suballocationOffset);

        void createNewSuballocationForBuffer(const std::shared_ptr<Buffer> &bufferToSuballocate,
                                             unsigned long long int suballocationOffset);

        void createVulkanBufferIfItDoesNotAlreadyExist(unsigned long long int newBufferSize);

        void validateNewBufferSize(unsigned long long int newBufferSize) const;

        void suballocateSceneBuffers(const std::vector<std::shared_ptr<Buffer>> &buffers);

        void defragmentIfNeeded();

        void computeCurrentAmountOfFragmentation();

        void validateGreatestMemoryBlockOffset(unsigned long greatestUsedMemoryBlockOffset);

        void defragmentUsedBlocksList();

        void defragmentFreeBlockList();

        void
        checkThatRequestedBufferUidsArePresent(const std::vector<PUtilities::UniqueIdentifier> &suballocatedBufferUids);
    };

} // PGraphics
