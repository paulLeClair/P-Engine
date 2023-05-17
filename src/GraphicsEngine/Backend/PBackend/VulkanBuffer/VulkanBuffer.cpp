//
// Created by paull on 2022-06-25.
//

#include "VulkanBuffer.hpp"

#include <stdexcept>

namespace PGraphics {

    VulkanBuffer::VulkanBuffer(const VulkanBuffer::CreationInput &createInfo) : vmaAllocator(createInfo.vmaAllocator),
                                                                                buffer(nullptr),
                                                                                bufferAllocation(nullptr),
                                                                                vmaAllocationIsMappable(
                                                                                        createInfo.makeVmaAllocationMappable),
                                                                                vmaAllocationHostAccessIsSequential(
                                                                                        createInfo.isVmaAllocationHostAccessSequential),
                                                                                memoryIsCurrentlyMapped(false) {
        VkBufferCreateInfo info = {
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                nullptr,
                0,// no sparse buffers yet, maybe that should be a separate implementation of VulkanBuffer?
                createInfo.bufferSize,
                createInfo.bufferUsages,
                VK_SHARING_MODE_EXCLUSIVE,// hardcoding exclusive sharing only for now
                static_cast<uint32_t>( createInfo.queueFamilyIndices.size()),
                createInfo.queueFamilyIndices.data()};

        VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
        vmaAllocationCreateInfo.usage = createInfo.vmaMemoryUsages;

        if (vmaAllocationIsMappable) {
            vmaAllocationCreateInfo.flags |= (vmaAllocationHostAccessIsSequential)
                                             ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                             : VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        }

        if (vmaCreateBuffer(vmaAllocator, &info, &vmaAllocationCreateInfo, &buffer, &bufferAllocation, nullptr) !=
            VK_SUCCESS) {
            throw std::runtime_error("Unable to create VkBuffer!");
        }
    }

    VulkanBuffer::~VulkanBuffer() {
        vmaDestroyBuffer(vmaAllocator, buffer, bufferAllocation);
    }


    VkBuffer VulkanBuffer::getBuffer() const {
        return buffer;
    }


    VmaAllocation VulkanBuffer::getBufferAllocation() const {
        return bufferAllocation;
    }


}// namespace PGraphics