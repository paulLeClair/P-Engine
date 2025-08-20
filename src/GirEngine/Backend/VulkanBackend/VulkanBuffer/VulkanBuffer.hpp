//
// Created by paull on 2022-06-25.
//
#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vector>
#include <stdexcept>

#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * This will be the class that wraps a Vulkan-facing buffer, which should be generated from a Scene buffer at
     * bakeRenderData time
     *
     * For now, it'll just accept the pre-calculated data that should be appropriate for some Vulkan buffer. The actual
     * logic for figuring out what this data should be happens in the VulkanFrameExecutionController's bakeRenderData process, and it's determined
     * mostly from the high-level information contained in the Scene specification
     *
     */
    class VulkanBuffer {
    public:
        struct CreationInput {
            // all of this info should be provided at bakeRenderData time...
            VmaAllocator vmaAllocator;

            VkDeviceSize bufferSize;
            VkBufferUsageFlags bufferUsages;
            std::vector<uint32_t> queueFamilyIndices;
            VmaMemoryUsage vmaMemoryUsages;

            bool makeVmaAllocationMappable = true;
            bool isVmaAllocationHostAccessSequential = true;
        };

        explicit VulkanBuffer(const CreationInput &createInfo);

        ~VulkanBuffer();

        [[nodiscard]] VkBuffer &getBuffer();

        [[nodiscard]] VmaAllocation &getBufferAllocation();

        bool mapVulkanBufferMemoryToPointer(void **dataAccessPointer) const {
            // TODO - I think we need to be invalidating the cache here!
            // to do this, we'd use: vmaInvalidateAllocation()

            return vmaMapMemory(vmaAllocator, bufferAllocation, dataAccessPointer) == VK_SUCCESS;
        }

        void unmapVulkanBufferMemory() const {
            // TODO - I think we need to be flushing the cache here!
            // to do this we'd use: vmaFlushAllocation()

            vmaUnmapMemory(vmaAllocator, bufferAllocation);
        }

    private:
        VmaAllocator vmaAllocator;

        VkBuffer buffer;
        VmaAllocation bufferAllocation;

        bool memoryIsCurrentlyMapped = false;

        bool vmaAllocationIsMappable;
        bool vmaAllocationHostAccessIsSequential;
    };
} // namespace PGraphics
