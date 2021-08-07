#pragma once

#include "../../PRender.hpp"

#include "./RenderResource.hpp"
#include "../../backend/resources/Image.hpp"

// this hopefully will be a general-purpose "image" resource
class ImageResource : public RenderResource {
  public:
    ImageResource(unsigned int index, const std::string &name, const AttachmentInfo &info);
    ~ImageResource() = default;

    void setTransientState(bool newValue) {
        _isTransient = newValue;
    }

    void addImageUsage(VkImageUsageFlags flags) {
        _imageUsage = _imageUsage | flags;
    }

    VkImageUsageFlags getImageUsageFlags() const {
        return _imageUsage;
    }

    AttachmentInfo &getAttachmentInfo() {
        return _attachmentInfo;
    }

    bool getTransientState() {
        return _isTransient;
    }

  protected:
    AttachmentInfo _attachmentInfo;
    VkImageUsageFlags _imageUsage = 0;
    bool _isTransient = false;

    // any other high-level image resource information ...

};