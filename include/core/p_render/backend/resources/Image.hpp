#pragma once

#include "../../PRender.hpp"

#include "./Resource.hpp"

#include "../../../../vulkan_memory_allocator/vk_mem_alloc.h"

// gonna restructure this to be similar to Granite, using an ImageCreateInfo struct which
// will also have some common usage cases provided as convenience functions; 

namespace Backend {

class Image : public Resource {
  public:
    enum ImageDomain {
        Physical,
        Transient
        // any others... for now i'm Keeping It Simple, Stupid
    };

    // some quick utilities
    static bool formatHasStencilAspect(VkFormat format) {

        switch (format) {
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            case VK_FORMAT_S8_UINT:
            case VK_FORMAT_D16_UNORM:
                return true;
        }

        return false;
    }

    static bool formatHasDepthAspect(VkFormat format) {
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;
        }

        return false;
    }

    static bool formatHasDepthOrStencilAspect(VkFormat format) {
        return formatHasDepthAspect(format) || formatHasStencilAspect(format);
    }

    struct ImageCreateInfo {
        // here we have all the info required to create a vulkan image, although
        // since i'm using VulkanMemoryAllocator i might be able to modify/truncate some stuff
        // however, id think you'd need this info even just to use VMA
        std::shared_ptr<Context> context = nullptr;
        VmaAllocator allocator = nullptr;

        ImageDomain domain = ImageDomain::Physical; // default to physical
        unsigned int width = 0;
        unsigned int height = 0;
        unsigned int depth = 1;
        unsigned int levels = 1; 

        VkFormat format = VK_FORMAT_UNDEFINED;
        VkImageType type = VK_IMAGE_TYPE_2D; // default to 2d images

        unsigned int layers = 1; // i'll go through and add this later, for now we won't support layering images

        VkImageUsageFlags usage = 0;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT; // default to 1 sample
        VkImageCreateFlags flags = 0;

        // could add in a "MiscFlags" enum here like Themaister but my engine is much simpler in scope 
        // and i can't think of anything i'd need to add currently

        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkComponentMapping swizzle = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        
        // whatever else is req'd....

        /* IMAGE PRESETS */
        static ImageCreateInfo renderTarget(std::shared_ptr<Context> context, VmaAllocator allocator, unsigned int width, unsigned int height, VkFormat format) {
            ImageCreateInfo info;

            info.context = context;
            info.allocator = allocator;

            info.width = width;
            info.height = height;
            info.depth = 1;
            info.levels = 1;
            info.type = VK_IMAGE_TYPE_2D;
            info.layers = 1;
            info.usage = (formatHasDepthOrStencilAspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                            | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            info.samples = VK_SAMPLE_COUNT_1_BIT;

            info.flags = 0;
            info.initialLayout = formatHasDepthOrStencilAspect(format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            return info;
        }

        // any other presets (TODO)
    };

    Image(ImageCreateInfo &imageCreateInfo) {
        assert(imageCreateInfo.context && imageCreateInfo.allocator);
        _context = imageCreateInfo.context;
        _allocator = imageCreateInfo.allocator;

        VkImageCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        
        assert(imageCreateInfo.format != VK_FORMAT_UNDEFINED);
        info.format = imageCreateInfo.format;

        info.imageType = VK_IMAGE_TYPE_2D; // only option for now
        info.initialLayout = imageCreateInfo.initialLayout;
        
        // TODO: allow for mipmapping
        info.mipLevels = 1;
        
        // TODO: implement compute stuff
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;

        info.samples = imageCreateInfo.samples;

        info.tiling = VK_IMAGE_TILING_OPTIMAL; // just default to optimal for now

        info.usage = imageCreateInfo.usage;

        info.arrayLayers = 1; // no array layers for now

        info.extent.width = imageCreateInfo.width;
        info.extent.height = imageCreateInfo.height;
        info.extent.depth = 1;

        // create the actual image thru VMA
        VmaAllocationCreateInfo allocCreateInfo = {};
        
        // customizing the VMA allocation params through the create info will probably be a good idea,
        // i still need to look into the details of VMA to figure out the correct way to use it for my purposes
            // i think i could differentiate here based on the domain but i'll start with just implementing GPU resources
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY; 

        // store info after it has been filled
        _info = info;

        if (vmaCreateImage(_allocator, &info, &allocCreateInfo, &_image, &_allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create image!");
        }
    }
    
    ~Image() {
        // should destroy the image!
        vmaDestroyImage(_allocator, _image, _allocation);
    }

    bool isImage() const override {
        return true;
    }

    const VkImage &getImage() {
        return _image;
    }

    // similarly to buffer, should probably make sure that all usages are known + set before
    // the Backend::Image is created so that this works
    VkImageUsageFlags getUsages() {
        return _info.usage;
    }

  protected:
    Image() {
        // no args to ctor -> only for protected classes that don't use the default mechanism of creating
        // an image in the ctor
    }

    std::shared_ptr<Context> _context = nullptr;

    VkImageCreateInfo _info = {}; // store info that image was created with

    VmaAllocator _allocator = nullptr;

    // each image should probably wrap a vkimage
    VkImage _image = VK_NULL_HANDLE;

    // we can stick the VMA thing here too
    VmaAllocation _allocation = nullptr;
};

}