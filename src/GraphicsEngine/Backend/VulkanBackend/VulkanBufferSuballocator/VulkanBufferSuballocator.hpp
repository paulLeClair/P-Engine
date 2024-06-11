//
// Created by paull on 2023-01-13.
//

#pragma once

#include <vector>

#include <vulkan/vulkan_core.h>

#include <boost/container/set.hpp>
#include <boost/container/flat_set.hpp>

#include "../../../GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp"
#include "../VulkanBuffer/VulkanBuffer.hpp"
#include "VulkanBufferSuballocation/VulkanBufferSuballocation.hpp"

namespace pEngine::girEngine::backend::vulkan {
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

            const std::vector<std::shared_ptr<gir::BufferIR> > &initialBuffers;

            /**
             * This is the maximum percentage of fragmentation that is allowed before the suballocator will initiate
             * defragmentation, where fragmentation is just the number of fragmented empty bytes divided by the total
             * of number bytes in the buffer
             */
            float defragmentationThreshold = 0.3f;

            // TODO - evaluate whether to include a max size and if so how to implement that

            std::vector<unsigned> queueFamilyIndices;

            bool disableTransferDestinationBufferUsage = false;
            bool disableTransferSourceBufferUsage = false;
        };

        explicit VulkanBufferSuballocator(const CreationInput &creationInput)
            : bufferUsageFlags(
                  // basically we just ensure that transfer source and destination usages are enabled unless specifically disabled
                  getInitialBufferUsageFlags(creationInput.disableTransferDestinationBufferUsage,
                                             creationInput.disableTransferSourceBufferUsage)
                  | creationInput.bufferUsageFlags
              ),
              defragmentationThreshold(creationInput.defragmentationThreshold),
              disableTransferDestinationBufferUsage(creationInput.disableTransferDestinationBufferUsage),
              disableTransferSourceBufferUsage(creationInput.disableTransferSourceBufferUsage),
              allocator(creationInput.allocator),
              bufferSize(0u),
              currentDefragmentationAmount(0u) {
            // since we'll generally want to be able to copy data to/from buffers, the new approach will be to
            // have the user specifically enable/disable transfer operations with some simple bool flags.
            // i can modify this pretty easily later but for now it should be fine

            suballocateBuffers(creationInput.initialBuffers);
        }

        ~VulkanBufferSuballocator() = default;

        void suballocateBuffers(const std::vector<std::shared_ptr<gir::BufferIR> > &buffersToSuballocate);

        void suballocateBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate);

        void freeSuballocatedBuffer(const util::UniqueIdentifier &suballocatedBufferUid);

        void freeSuballocationsByUid(const std::vector<util::UniqueIdentifier> &suballocatedBufferUids);

        void freeAllBufferSuballocations();

        // TODO - modify this so that it doesn't use a unique identifier (?)
        [[nodiscard]] const VulkanBufferSuballocation *findSuballocation(
            const util::UniqueIdentifier &suballocationUid) const {
            for (auto &suballocation: suballocations) {
                if (suballocation->getUniqueIdentifier() == suballocationUid) {
                    return suballocation.get();
                }
            }
            return nullptr;
        }

        [[nodiscard]] const VkBuffer &getBufferHandle() const {
            return buffer->getBuffer();
        }

    private:
        VmaAllocator allocator;

        std::vector<unsigned> queueFamilyIndices;

        /**
         * This is the buffer that the VulkanBufferSuballocations are stored in; it will be dynamically
         * resized when it grows beyond its current capacity, and will automatically defragment once
         * it detects that the current setup is
         */
        std::shared_ptr<VulkanBuffer> buffer;

        std::vector<std::shared_ptr<VulkanBufferSuballocation> > suballocations = {};
        std::unordered_map<util::UniqueIdentifier, size_t> suballocationUniqueIds;

        const VkBufferUsageFlags bufferUsageFlags;

        const float defragmentationThreshold;

        const bool disableTransferSourceBufferUsage;
        const bool disableTransferDestinationBufferUsage;

        size_t bufferSize;

        struct MemoryBlock {
            size_t offset;
            size_t size;

            util::UniqueIdentifier suballocationUid;

            bool operator<(const MemoryBlock &other) const {
                return offset < other.offset;
            }

            MemoryBlock(const size_t &offset, const size_t &size, const util::UniqueIdentifier &suballocation_uid)
                : offset(offset),
                  size(size),
                  suballocationUid(suballocation_uid) {
            }
        };

        boost::container::flat_set<MemoryBlock, std::less<> > freeBlocks = {};
        boost::container::flat_set<MemoryBlock, std::less<> > suballocatedBlocks = {};

        double currentDefragmentationAmount;

        static VkBufferUsageFlags getInitialBufferUsageFlags(const bool disableTransferDestination,
                                                             const bool disableTransferSource) {
            VkBufferUsageFlags usageFlags = 0;
            if (!disableTransferDestination) {
                usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            }
            if (!disableTransferSource) {
                usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
            return usageFlags;
        }

        void defragmentBuffer();

        void createVulkanBuffer();

        void suballocateBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
                               unsigned long long suballocationOffset);

        [[nodiscard]] size_t
        computeNewBufferSize(const std::vector<std::shared_ptr<gir::BufferIR> > &buffersToSuballocate) const;

        void copyBufferDataToMappedVulkanBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
                                                size_t suballocationOffset);

        void createNewSuballocationForBuffer(const std::shared_ptr<gir::BufferIR> &bufferToSuballocate,
                                             size_t suballocationOffset);

        void createVulkanBufferIfItDoesNotAlreadyExist(size_t newBufferSize);

        // void validateNewBufferSize(size_t newBufferSize) const;

        void suballocateBufferList(const std::vector<std::shared_ptr<gir::BufferIR> > &buffers);

        void defragmentIfNeeded();

        void computeCurrentAmountOfFragmentation();

        void validateGreatestMemoryBlockOffset(size_t greatestUsedMemoryBlockOffset) const;

        void defragmentUsedBlocksList();

        void defragmentFreeBlockList();

        void
        checkThatRequestedBufferUidsArePresent(const std::vector<util::UniqueIdentifier> &suballocatedBufferUids);
    };
} // PGraphics
