//
// Created by paull on 2022-06-25.
//

#include "VulkanImage.hpp"

namespace pEngine::girEngine::backend::vulkan {
    [[maybe_unused]] VkImage VulkanImage::getImage() const {
        return image;
    }

    VmaAllocation VulkanImage::getImageAllocation() const {
        return imageAllocation;
    }

    const VkImageView &VulkanImage::getImageView() const {
        return view;
    }
} // namespace PGraphics
