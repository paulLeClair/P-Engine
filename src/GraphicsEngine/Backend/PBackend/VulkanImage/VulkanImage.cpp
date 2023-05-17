//
// Created by paull on 2022-06-25.
//

#include "VulkanImage.hpp"

namespace PGraphics {

    [[maybe_unused]] VkImage VulkanImage::getImage() const {
        return image;
    }

    VmaAllocation VulkanImage::getImageAllocation() const {
        return imageAllocation;
    }

}// namespace PGraphics