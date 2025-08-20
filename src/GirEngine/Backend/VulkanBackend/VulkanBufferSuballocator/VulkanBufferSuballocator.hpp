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
#include "VulkanBufferSuballocation/VulkanBufferSuballocationHandle.hpp"

namespace pEngine::girEngine::backend::vulkan {
    /**
     *
     */
    class VulkanBufferSuballocator {
    public:
        struct CreationInput {
            VmaAllocator allocator;

            VkBufferUsageFlags bufferUsageFlags;

            const std::vector<gir::BufferIR> &initialBuffers;

            std::vector<unsigned> queueFamilyIndices;

            uint32_t minimumBufferAlignment = 1;
            /**
             * This is the maximum percentage of fragmentation that is allowed before the suballocator will initiate
             * defragmentation, where fragmentation is just the number of fragmented empty bytes divided by the total
             * of number bytes in the buffer
             */
            float defragmentationThreshold = 0.3f;

            // TODO - evaluate whether to include a max size and if so how to implement that

            bool disableTransferDestinationBufferUsage = false;
            bool disableTransferSourceBufferUsage = false;
        };

        explicit VulkanBufferSuballocator(const CreationInput &creationInput)
            : allocator(creationInput.allocator),
              bufferUsageFlags(
                  // basically we just ensure that transfer source and destination usages are enabled unless specifically disabled
                  getInitialBufferUsageFlags(creationInput.disableTransferDestinationBufferUsage,
                                             creationInput.disableTransferSourceBufferUsage)
                  | creationInput.bufferUsageFlags),
              defragmentationThreshold(creationInput.defragmentationThreshold),
              disableTransferSourceBufferUsage(creationInput.disableTransferSourceBufferUsage),
              disableTransferDestinationBufferUsage(creationInput.disableTransferDestinationBufferUsage),
              minimumBufferAlignment(creationInput.minimumBufferAlignment),
              bufferSize(0u),
              currentDefragmentationAmount(0u) {
            // since we'll generally want to be able to copy data to/from buffers, the new approach will be to
            // have the user specifically enable/disable transfer operations with some simple bool flags.
            // i can modify this pretty easily later but for now it should be fine
            allocator = creationInput.allocator;

            // maybe we don't need this...
            // suballocateBuffers(creationInput.initialBuffers);
        }

        VulkanBufferSuballocator() = default;

        ~VulkanBufferSuballocator() = default;

        VulkanBufferSuballocator(const VulkanBufferSuballocator &other) = default;

        void suballocateBuffers(const std::vector<gir::BufferIR> &buffersToSuballocate);

        void suballocateBuffer(const gir::BufferIR &bufferToSuballocate);

        void freeSuballocatedBuffer(const util::UniqueIdentifier &suballocatedBufferUid);

        void freeSuballocationsByUid(const std::vector<util::UniqueIdentifier> &suballocatedBufferUids);

        void freeAllBufferSuballocations();

        /**
         * I was initially thinking to just try and leverage the heck outta these vulkan buffer suballocations;
         * I think that will work but it'll take modifications, or maybe a slight re-route into a new type that
         * will provide information needed to actually bind a particular buffer suballocation to a command buffer,
         * or whatever other shenanigans you might want to do with them.
         *
         * One way to avoid a new type though could be to have a static convenience method that takes a suballocator and a
         * suballocation and gives you a struct with the vulkan buffer handle and the offset (plus anything else needed)
         * for manipulating the buffer in Vulkan.
         */
        [[nodiscard]] boost::optional<VulkanBufferSuballocationHandle> findSuballocation(
            const util::UniqueIdentifier &suballocationUid) {
            if (!suballocations.count(suballocationUid)) {
                return boost::none;
            }
            return {
                obtainHandleForSuballocation(suballocations.at(suballocationUid))
            };
        }

        [[nodiscard]] const VkBuffer &getBufferHandle() const {
            return buffer->getBuffer();
        }

        [[nodiscard]] VmaAllocation &getAllocation() {
            return allocation;
        }

    private:
        VmaAllocator allocator = VK_NULL_HANDLE;

        std::vector<unsigned> queueFamilyIndices = {};

        /**
         * This is the buffer that the VulkanBufferSuballocations are stored in; it will be dynamically
         * resized when it grows beyond its current capacity, and will automatically defragment once
         * it detects that the current setup is
         */
        std::shared_ptr<VulkanBuffer> buffer;

        VmaAllocation allocation = VK_NULL_HANDLE;

        /**
         *
         */
        std::unordered_map<util::UniqueIdentifier, VulkanBufferSuballocation> suballocations = {};

        VkBufferUsageFlags bufferUsageFlags;

        float defragmentationThreshold;

        bool disableTransferSourceBufferUsage;
        bool disableTransferDestinationBufferUsage;

        // HOME STRETCH (model demo):
        // we now have to do a minor refactor of this damn suballocation stuff, where we just need to be
        // respecting this "minimum buffer alignment" thing; this just means we have to allocate our memory
        // in chunks of a certain size, and pad the last element
        // ONE THING: we're suballocating into sub-chunks which have different sizes, so we may have to break this down
        // so that we're padding the end of each allocated buffer and aligning them per-buffer in that way
        uint32_t minimumBufferAlignment;

        size_t bufferSize;

        /**
         * This is the device memory block
         * for a given suballocation (identified by the uid here)
         */
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

        /**
         * Given an existing suballocation, it should provide the handle that the external resource
         * needs to be able to access the relevant device memory for drawing etc
         */
        [[nodiscard]] VulkanBufferSuballocationHandle
        obtainHandleForSuballocation(VulkanBufferSuballocation &suballocation) {
            if (!buffer || suballocation.resourceId.getValue().is_nil()) {
                return {};
            }
            return {
                suballocation,
                buffer->getBuffer()
            };
        }

        void defragmentBuffer();

        void createVulkanBuffer();

        void suballocateBuffer(const gir::BufferIR &bufferToSuballocate,
                               size_t suballocationOffset);

        [[nodiscard]] size_t
        computeNewBufferSize(const std::vector<gir::BufferIR> &buffersToSuballocate) const;

        void copyBufferDataToMappedVulkanBuffer(const gir::BufferIR &bufferToSuballocate,
                                                size_t suballocationOffset);

        void createNewSuballocationForBuffer(const gir::BufferIR &bufferToSuballocate,
                                             size_t suballocationOffset);

        void createVulkanBufferIfNonexistent(size_t newBufferSize);

        // void validateNewBufferSize(size_t newBufferSize) const;

        void suballocateBufferList(const std::vector<gir::BufferIR> &buffers);

        void defragmentIfNeeded();

        void computeCurrentAmountOfFragmentation();

        void validateGreatestMemoryBlockOffset(size_t greatestUsedMemoryBlockOffset) const;

        void defragmentUsedBlocksList();

        void defragmentFreeBlockList();

        void
        checkThatRequestedBufferUidsArePresent(const std::vector<util::UniqueIdentifier> &suballocatedBufferUids);
    };
} // PGraphics
