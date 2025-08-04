//
// Created by paull on 2022-06-25.
//

#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <boost/optional.hpp>

#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * This will be the class that wraps a Vulkan-facing image, which should be generated from a Scene image at bakeRenderData
     * time
     *
     * TODO -> for model demo, make it so that no allocations happen if you specify an image as a swapchain image
     *
     */
    class VulkanImage {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
            VkDevice device;
            VkImageType imageType;
            VkFormat imageFormat;
            VkExtent3D imageExtent;
            uint32_t numberOfMipLevels;
            uint32_t numberOfArrayLayers;
            VkSampleCountFlagBits samples;
            VkImageTiling tiling;
            VkImageUsageFlags imageUsages;
            std::vector<uint32_t> queueFamilyIndices;
            VkImageLayout initialImageLayout;
            VmaMemoryUsage vmaMemoryUsages;
            VmaAllocator allocator;
            boost::optional<VkImage> swapchainImageHandle = boost::none;
            boost::optional<uint32_t> swapchainImageIndex = boost::none;
        };

        VulkanImage() = default;

        friend bool operator==(const VulkanImage &lhs, const VulkanImage &rhs) {
            return lhs.width == rhs.width
                   && lhs.height == rhs.height
                   && lhs.depth == rhs.depth
                   && lhs.imageFormat == rhs.imageFormat
                   && lhs.name == rhs.name
                   && lhs.uid == rhs.uid
                   && lhs.swapchainImageIndex == rhs.swapchainImageIndex
                   && lhs.image == rhs.image
                   && lhs.view == rhs.view
                   && lhs.allocator == rhs.allocator
                   && lhs.imageAllocation == rhs.imageAllocation;
        }

        friend bool operator!=(const VulkanImage &lhs, const VulkanImage &rhs) {
            return !(lhs == rhs);
        }

        explicit VulkanImage(const CreationInput &createInfo) : width(createInfo.imageExtent.width),
                                                                height(createInfo.imageExtent.height),
                                                                depth(createInfo.imageExtent.depth),
                                                                imageFormat(createInfo.imageFormat),
                                                                name(createInfo.name),
                                                                uid(createInfo.uid),
                                                                swapchainImageIndex(createInfo.swapchainImageIndex),
                                                                image(nullptr),
                                                                allocator(createInfo.allocator),
                                                                imageAllocation(nullptr) {
            // TODO -> we need to make sure the case where this is a swapchain image is being handled properly;
            // I'm not even sure if that might warrant a whole new abstraction for it

            if (!createInfo.swapchainImageHandle.has_value()) {
                VkImageCreateInfo imageCreateInfo = {
                    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    nullptr,
                    0,
                    createInfo.imageType,
                    createInfo.imageFormat,
                    createInfo.imageExtent,
                    createInfo.numberOfMipLevels,
                    createInfo.numberOfArrayLayers,
                    createInfo.samples,
                    createInfo.tiling,
                    createInfo.imageUsages,
                    VK_SHARING_MODE_EXCLUSIVE, // hardcoded exclusive sharing mode for now
                    static_cast<uint32_t>(createInfo.queueFamilyIndices.size()),
                    createInfo.queueFamilyIndices.data(),
                    VK_IMAGE_LAYOUT_UNDEFINED
                    // apparently this can only be undefined or preinit'd layout, all others are wrong
                };

                VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
                vmaAllocationCreateInfo.usage = createInfo.vmaMemoryUsages;

                if (const VkResult result =
                            vmaCreateImage(allocator,
                                           &imageCreateInfo,
                                           &vmaAllocationCreateInfo,
                                           &image,
                                           &imageAllocation,
                                           nullptr);
                    result != VK_SUCCESS) {
                    throw std::runtime_error("Unable to create VkImage!");
                }
            } else {
                image = createInfo.swapchainImageHandle.get();
            }
            // can we just build the image view once? or should we be building & returning based off certain info?

            // TODO -> break this off into a separate function and have it support all the different usages
            VkImageAspectFlags imageAspectFlags = 0;
            if (createInfo.imageUsages & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
                imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
            } else if (createInfo.imageUsages & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
            }

            VkImageViewCreateInfo imageViewCreateInfo{
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                nullptr,
                0,
                image,
                VK_IMAGE_VIEW_TYPE_2D, // TODO -> support other kinds of images
                createInfo.imageFormat,
                VkComponentMapping{
                    // default component mapping
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
                },
                VkImageSubresourceRange{
                    imageAspectFlags,
                    // TODO -> obtain this dynamically for non-color images
                    // TODO: support arrays and mipmap
                    0,
                    VK_REMAINING_MIP_LEVELS,
                    0,
                    VK_REMAINING_ARRAY_LAYERS
                }
            };
            if (
                const VkResult result = vkCreateImageView(createInfo.device, &imageViewCreateInfo, nullptr, &view);
                result != VK_SUCCESS) {
                throw std::runtime_error("Unable to create image view!");
            }
        }

        ~VulkanImage() {
            if (!swapchainImageIndex.has_value()) {
                vmaDestroyImage(allocator, image, imageAllocation);
            }
        }

        [[maybe_unused]] [[nodiscard]] VkImage getImage() const;

        [[nodiscard]] VmaAllocation getImageAllocation() const;

        const VkImageView &getImageView() const;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;

        VkFormat imageFormat = VK_FORMAT_UNDEFINED;
        std::string name;
        util::UniqueIdentifier uid;

        boost::optional<uint32_t> swapchainImageIndex = boost::none;

    private:
        VkImage image = VK_NULL_HANDLE;

        VkImageView view = VK_NULL_HANDLE;

        VmaAllocator allocator = VK_NULL_HANDLE;
        VmaAllocation imageAllocation = VK_NULL_HANDLE;
    };
} // namespace PGraphics
