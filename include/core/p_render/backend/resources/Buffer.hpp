#pragma once

#include "./Resource.hpp"

#include "../../../../vulkan_memory_allocator/vk_mem_alloc.h"

namespace backend {

class Buffer : public Resource {
  public:
    // enum for the memory domain of this buffer (default to device but this should be a way to implement the others as well)
    enum class BufferDomain {
        // a lot of these i have to look into the specifics; for most rendering uses i think device is good for now,
        // but i'll try and build support for these asap
        Device,
        LinkedDeviceHost,
        LinkedDeviceHostDevicePreferred,
        Host,
        CachedHost,
        CachedCoherentHostCoherentPreferred,
        CachedCoherentHostCachedPreferred
    };

    struct BufferCreateInfo {
        VmaAllocator allocator = nullptr;
        BufferDomain domain = BufferDomain::Device; // by default it'll be a VRAM buffer
        VkDeviceSize size = 0;
        VkBufferUsageFlags usages = 0;
        // could add some misc flags for later 
    };

    Buffer(BufferCreateInfo &createInfo) : info_(createInfo), allocator_(info_.allocator) {
        // we should allocate the buffer here i think! then
        // it should be able to be used in rendering

        VkBufferCreateInfo info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        
        // set sharing mode to exclusive for now, but could support multiple queues in the future
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;

        info.size = info_.size;
        info.usage = info_.usages;

        VmaAllocationCreateInfo allocCreateInfo = {};
        
        // TODO - make this configurable thru the BufferCreateInfo
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; 

        if (vmaCreateBuffer(allocator_, &info, &allocCreateInfo, &buffer_, &allocation_, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create buffer!");
        }
        
    }

    ~Buffer() {
        vmaDestroyBuffer(allocator_, buffer_, allocation_);
    }

    bool isBuffer() const override {
        return true;
    }

    // i think we gotta make sure all usages are set before this backend::Buffer object is created
    VkBufferUsageFlags getUsages() {
        return info_.usages;
    }

  private:
    BufferCreateInfo info_;

    VkBuffer buffer_;

    // vma stuff
    VmaAllocator allocator_;
    VmaAllocation allocation_;

};

}