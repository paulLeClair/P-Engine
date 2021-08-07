#pragma once

#include "../../PRender.hpp"
#include "./ResourceDimensions.hpp"

// this is another thing that the render graph needs:

enum ResourceQueueUsageFlagBits {
    GRAPHICS_QUEUE = 1 << 0,
    COMPUTE_QUEUE = 1 << 1
};
using ResourceQueueUsageFlags = uint32_t;

// image size classes (similar to Themaister)
enum class ImageSizeClass {
    Absolute, // absolute size (should be used for some things)
    SwapchainRelative, // float parameter (i think b/w 0 and 1) relative to swapchain image size 
    InputRelative // float parameter relative to input image size (should be used for some things)
};

// image resource aux info
struct AttachmentInfo {
    // size class (which determines how to handle dimensions etc)
    ImageSizeClass sizeClass = ImageSizeClass::SwapchainRelative;
    
    // image dimensions
    float size_x = 1.0f;
    float size_y = 1.0f;
    float size_z = 0.0f; // 2d images by default

    // some vulkan-specific format info
    VkFormat format = VK_FORMAT_UNDEFINED;

    // number of samples
    unsigned int numSamples = 1;

    // number of mip map levels (i think)
    unsigned int numLevels = 1;

    // number of layers (since vulkan images can be interleaved if you want)
    unsigned int numLayers = 1;

    // image usage flags (auxiliary)
    VkImageUsageFlags auxUsage = 0;

    // misc flags:
    bool persistent = true;
    bool transient = false;

    // name of input image for relative sizing
    std::string sizeRelativeName;

};

struct BufferInfo {
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    bool persistent = true;

    bool operator==(const BufferInfo &other) {
        return size == other.size && usage == other.usage && persistent == other.persistent;
    }

    bool operator!=(const BufferInfo &other) {
        return !(*this == other);
    }

};

struct ResourceDimensions {
    ~ResourceDimensions() = default;
    // this is the generic structure for encoding a single physical resourcee (which may correspond to 
    // a few render graph resources)
        // we distinguish between images and buffers by testing the buffer size (will be nonzero for a buffer)
    // i guess we could also distinguish them by just storing the RenderResource::Type here...
    VkFormat format = VK_FORMAT_UNDEFINED;
    BufferInfo bufferInfo = {};

    // dimensions info:
    unsigned width = 0;
    unsigned height = 0;
    unsigned depth = 1; 
    unsigned layers = 1;
    unsigned mipLevels = 1;
    unsigned samples = 1;

    bool transient = false;
    bool unormSRGB = false;
    bool persistent = true;
    bool proxy = false;

    VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    ResourceQueueUsageFlags queueUsages  = 0;
    VkImageUsageFlags imageUsage = 0;

    bool operator==(const ResourceDimensions &other) const {
        return format == other.format &&
            width == other.width &&
            height == other.height &&
            depth == other.depth &&
            layers == other.layers &&
            mipLevels == other.mipLevels && 
            bufferInfo.size == other.bufferInfo.size &&
            bufferInfo.usage == other.bufferInfo.usage &&
            bufferInfo.persistent == other.persistent &&
            transient == other.transient &&
            persistent == other.persistent &&
            unormSRGB == other.unormSRGB &&
            transform == other.transform &&
            proxy == other.proxy;
    }

    bool operator!=(const ResourceDimensions &other) const {
        return !(*this == other);
    }

    bool isStorageImage() const {
        return (imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0;
    }

    bool isBufferLike() const {
        return isStorageImage() || (bufferInfo.size != 0) || proxy;
    }

    std::string name = "";
};

