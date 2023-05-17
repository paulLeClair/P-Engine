//
// Created by paull on 2022-06-25.
//

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <utility>
#include <vector>

#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace PGraphics {

/**
 * This will be the class that wraps a Vulkan-facing image, which should be generated from a Scene image at bakeRenderData
 * time
 */
    class VulkanImage {
    public:
        struct CreationInput {
            std::string name;
            PUtilities::UniqueIdentifier uid;
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

            CreationInput(std::string name, const PUtilities::UniqueIdentifier &uid, VkImageType imageType,
                          VkFormat imageFormat, const VkExtent3D &imageExtent, uint32_t numberOfMipLevels,
                          uint32_t numberOfArrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling,
                          VkImageUsageFlags imageUsages, std::vector<uint32_t> queueFamilyIndices,
                          VkImageLayout initialImageLayout, VmaMemoryUsage vmaMemoryUsages,
                          VmaAllocator allocator) : name(std::move(name)), uid(uid), imageType(imageType),
                                                    imageFormat(imageFormat), imageExtent(imageExtent),
                                                    numberOfMipLevels(numberOfMipLevels),
                                                    numberOfArrayLayers(numberOfArrayLayers), samples(samples),
                                                    tiling(tiling), imageUsages(imageUsages),
                                                    queueFamilyIndices(std::move(queueFamilyIndices)),
                                                    initialImageLayout(initialImageLayout),
                                                    vmaMemoryUsages(vmaMemoryUsages), allocator(allocator) {}
        };

        explicit VulkanImage(const CreationInput &createInfo) : name(createInfo.name), uid(createInfo.uid),
                                                                allocator(createInfo.allocator), image(nullptr),
                                                                imageAllocation(nullptr) {
            VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
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
                                                 VK_SHARING_MODE_EXCLUSIVE,// hardcoded exclusive sharing mode for now
                                                 static_cast<uint32_t>( createInfo.queueFamilyIndices.size()),
                                                 createInfo.queueFamilyIndices.data(),
                                                 createInfo.initialImageLayout};

            VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
            vmaAllocationCreateInfo.usage = createInfo.vmaMemoryUsages;

            VkResult result =
                    vmaCreateImage(allocator, &imageCreateInfo, &vmaAllocationCreateInfo, &image, &imageAllocation,
                                   nullptr);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to create VkImage!");
            }
        }

        ~VulkanImage() {
            vmaDestroyImage(allocator, image, imageAllocation);
        }

        [[maybe_unused]] [[nodiscard]] VkImage getImage() const;

        [[nodiscard]] VmaAllocation getImageAllocation() const;

    private:
        std::string name;
        PUtilities::UniqueIdentifier uid;

        VkImage image;

        VmaAllocator allocator;
        VmaAllocation imageAllocation;
    };

}// namespace PGraphics
