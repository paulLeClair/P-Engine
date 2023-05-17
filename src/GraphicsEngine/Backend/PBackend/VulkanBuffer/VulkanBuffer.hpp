//
// Created by paull on 2022-06-25.
//
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vector>
#include <stdexcept>

#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"

namespace PGraphics {

/**
 * This will be the class that wraps a Vulkan-facing buffer, which should be generated from a Scene buffer at
 * bakeRenderData time
 *
 * For now, it'll just accept the pre-calculated data that should be appropriate for some Vulkan buffer. The actual
 * logic for figuring out what this data should be happens in the PFrameExecutionController's bakeRenderData process, and it's determined
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

        [[nodiscard]] VkBuffer getBuffer() const;

        [[nodiscard]] VmaAllocation getBufferAllocation() const;

        void *mapVulkanBufferMemory() {
            if (memoryIsCurrentlyMapped) {
                return dataAccessPointer;
            }

            memoryIsCurrentlyMapped = true;

            auto mapMemoryResult = vmaMapMemory(vmaAllocator, bufferAllocation, &dataAccessPointer);
            if (mapMemoryResult != VK_SUCCESS) {
                // TODO - log!
                throw std::runtime_error("Unable to map Vulkan buffer memory!");
            }
            return dataAccessPointer;
        }

        void unmapVulkanBufferMemory() {
            if (!memoryIsCurrentlyMapped) {
                // TODO - log!
                throw std::runtime_error("Unable to unmap Vulkan buffer memory!");
            }
            vmaUnmapMemory(vmaAllocator, bufferAllocation);
        }

    private:
        VmaAllocator vmaAllocator;

        VkBuffer buffer;
        VmaAllocation bufferAllocation;

        bool memoryIsCurrentlyMapped = false;
        void *dataAccessPointer;

        bool vmaAllocationIsMappable;
        bool vmaAllocationHostAccessIsSequential;
    };

}// namespace PGraphics
