#pragma once

#include "./RenderResource.hpp"

class BufferResource : public RenderResource {
  public:
    BufferResource(unsigned int index, const std::string &name, const BufferInfo &info);
    ~BufferResource() = default;

    // yay
    void addBufferUsage(VkBufferUsageFlags usages) {
        usages_ = usages_ | usages;
    }

    VkBufferUsageFlags getBufferUsages() const {
        return usages_;
    }

    void setBufferInfo(const BufferInfo &newInfo) {
        bufferInfo_ = newInfo;
    }

    const BufferInfo &getBufferInfo() const {
        return bufferInfo_;
    }

  private:
    BufferInfo bufferInfo_;
    VkBufferUsageFlags usages_ = 0;

};