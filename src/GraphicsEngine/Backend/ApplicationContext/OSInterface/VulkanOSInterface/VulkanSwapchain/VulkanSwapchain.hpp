//
// Created by paull on 2024-05-12.
//

#pragma once

#include <ranges>
#include <boost/optional.hpp>
#include <vulkan/vulkan.hpp>

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::appContext::osInterface::swapchain::vulkan {
    /**
     *
     */
    class VulkanSwapchain {
    public:
        struct CreationInput {
            VkInstance instance = VK_NULL_HANDLE;
            VkDevice logicalDevice = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR; // default to vsync?
            VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag;

            unsigned numberOfSwapchainImages = 3; // I'm assuming this would depend on the present mode

            VkExtent2D swapchainImageExtent; // TODO - determine if we can have this be determined automatically
            VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR swapchainImageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkImageUsageFlags swapchainImageUsageFlags = 0u;

            VkSurfaceKHR appWindowSurface;
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform =
                    VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

            bool isClipped = true;

            boost::optional<VkSwapchainKHR> oldSwapchain = boost::none;
        };


        explicit VulkanSwapchain(const CreationInput &creationInput)
            : instance(creationInput.instance),
              logicalDevice(creationInput.logicalDevice),
              physicalDevice(creationInput.physicalDevice),
              surface(creationInput.appWindowSurface),
              swapchainImageFormat(creationInput.swapchainImageFormat),
              swapchainPresentMode(creationInput.swapchainPresentMode),
              swapchainCompositeAlphaFlag(creationInput.swapchainCompositeAlphaFlag),
              swapchainImageColorSpace(creationInput.swapchainImageColorSpace),
              swapchainImageUsageFlags(creationInput.swapchainImageUsageFlags),
              swapchainImagePreTransform(creationInput.swapchainImagePreTransform),
              isClipped(creationInput.isClipped),
              oldSwapchain(creationInput.oldSwapchain) {
            // validate input extent
            maximumSwapchainImageExtent = getMaximumImageExtent();
            minimumSwapchainImageExtent = getMinimumImageExtent();
            if (!inputExtentIsValid(creationInput.swapchainImageExtent)) {
                // TODO - log!
                // default behavior for now: just set it to the minimum (re-evaluate this later)
                swapchainImageExtent = minimumSwapchainImageExtent;
            } else {
                swapchainImageExtent = creationInput.swapchainImageExtent;
            }

            VkSwapchainCreateInfoKHR swapchainCreateInfo{
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                nullptr,
                0,
                surface,
                creationInput.numberOfSwapchainImages,
                swapchainImageFormat,
                swapchainImageColorSpace,
                swapchainImageExtent,
                1, // TODO - support image arrays (i'm ignoring it for now)
                swapchainImageUsageFlags,
                VK_SHARING_MODE_EXCLUSIVE,
                // TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                0, // TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                nullptr, // TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                swapchainImagePreTransform,
                swapchainCompositeAlphaFlag,
                swapchainPresentMode,
                isClipped ? VK_TRUE : VK_FALSE,
                creationInput.oldSwapchain.get_value_or(VK_NULL_HANDLE)
            };

            if (vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr,
                                     &swapchain) != VK_SUCCESS) {
                // TODO - log intead of throw! maybe re-integrate error code return enums!
                throw std::runtime_error("Error in VulkanSwapchain() -> Unable to create swapchain!");
            }

            if (!acquireSwapchainImages()) {
                // TODO - log!
            }
        }

        ~VulkanSwapchain() = default;

        VkImage getSwapchainImage(unsigned imageIndex) const {
            // TODO - re-evaluate how this works; however, to keep it agnostic w.r.t present mode, an index-based access seems ok
            return swapchainImages[imageIndex];
        }

        VkImage getSwapchainImage(util::UniqueIdentifier imageId) const {
            const auto findResult = std::ranges::find_if(swapchainImageUids,
                                                         [&](const auto &pair) {
                                                             return pair.second == imageId;
                                                         });
            if (findResult == std::ranges::end(swapchainImageUids)) {
                // TODO -> log!
                return VK_NULL_HANDLE;
            }
            return findResult->first;
        }

        [[nodiscard]] const uint32_t &getSwapchainImageWidth() const {
            return swapchainImageExtent.width;
        }

        [[nodiscard]] const uint32_t &getSwapchainImageHeight() const {
            return swapchainImageExtent.height;
        }

        [[nodiscard]] VkSwapchainKHR &getSwapchain() {
            return swapchain;
        }

        void setSwapchainImageExtent(VkExtent2D newSwapchainImageExtent) {
            // TODO - static assert that the min/max are not null
            if (newSwapchainImageExtent.width > maximumSwapchainImageExtent.width ||
                newSwapchainImageExtent.height > maximumSwapchainImageExtent.height) {
                // TODO - log
                throw std::runtime_error(
                    "Error in setSwapchainImageExtent - new swapchain image extent is beyond the maximum!");
            }

            if (newSwapchainImageExtent.width < minimumSwapchainImageExtent.width ||
                newSwapchainImageExtent.height < minimumSwapchainImageExtent.height) {
                // TODO - log
                throw std::runtime_error(
                    "Error in setSwapchainImageExtent - new swapchain image extent is less than the minimum!");
            }

            swapchainImageExtent = newSwapchainImageExtent;
        }

        [[nodiscard]] unsigned getNumberOfSwapchainImages() {
            return swapchainImages.size();
        }

        [[nodiscard]] const VkExtent2D &getSwapchainImageExtent() const {
            return swapchainImageExtent;
        }

        [[nodiscard]] const std::vector<VkImage> &getSwapchainImages() const {
            return swapchainImages;
        }

        VkSwapchainKHR getHandle() {
            return swapchain;
        }

        util::UniqueIdentifier getSwapchainImageUniqueIdentifier(VkImage value) const {
            for (auto &image: swapchainImages) {
                if (value == image) {
                    return swapchainImageUids.at(image);
                }
            }
            return {};
        }


        VkImageView getSwapchainImageView(uint32_t index) {
            if (index >= swapchainImages.size()) {
                // TODO -> log!
                return VK_NULL_HANDLE;
            }
            return swapchainImageViews[index];
        }

        std::unordered_map<util::UniqueIdentifier, uint32_t> swapchainImageViewIndices;

        VkImageView getSwapchainImageView(util::UniqueIdentifier imageId) {
            const auto findResult = std::ranges::find_if(swapchainImageUids, [&](const auto &pair) {
                return pair.second == imageId;
            });
            if (findResult == std::ranges::end(swapchainImageUids)) {
                return VK_NULL_HANDLE;
            }
            return swapchainImageViews[swapchainImageViewIndices[findResult->second]];
        }

        // TODO - evaluate whether we should make it so that you can trigger recreation of the internal swapchain;
        // it seems kinda like cleaner design to just have the entire object be recreated if you're going to be trashing the original one anyway

        // these will just be public since they only provide information about the currently-created swapchain
        const VkPresentModeKHR swapchainPresentMode;
        const VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag;
        const VkFormat swapchainImageFormat;
        const VkColorSpaceKHR swapchainImageColorSpace;
        const VkImageUsageFlags swapchainImageUsageFlags;
        const VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform;
        const bool isClipped;
        const boost::optional<VkSwapchainKHR> oldSwapchain;

    private:
        VkInstance instance;
        VkDevice logicalDevice;
        VkPhysicalDevice physicalDevice;

        VkSwapchainKHR swapchain{};

        std::unordered_map<VkImage, util::UniqueIdentifier> swapchainImageUids = {};
        std::vector<VkImage> swapchainImages = {};
        std::vector<VkImageView> swapchainImageViews = {};

        VkExtent2D swapchainImageExtent{};

        VkExtent2D minimumSwapchainImageExtent{};
        VkExtent2D maximumSwapchainImageExtent{};

        VkSurfaceKHR surface = VK_NULL_HANDLE;

        // TODO - make sure this is all good
        bool acquireSwapchainImages() {
            uint32_t swapchainImageCount = 0;
            auto result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, nullptr);

            if (result != VK_SUCCESS) {
                // TODO - log!
                return false;
            }

            swapchainImages.resize(swapchainImageCount);
            result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount,
                                             swapchainImages.data());
            if (result != VK_SUCCESS) {
                return false;
            }

            for (auto &image: swapchainImages) {
                swapchainImageUids[image] = util::UniqueIdentifier();

                swapchainImageViews.push_back(VK_NULL_HANDLE);
                auto &view = swapchainImageViews.back();

                VkImageViewCreateInfo imageViewCreateInfo{
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    nullptr,
                    0,
                    image,
                    VK_IMAGE_VIEW_TYPE_2D,
                    swapchainImageFormat,
                    VkComponentMapping{
                        // default component mapping
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY,
                        VK_COMPONENT_SWIZZLE_IDENTITY
                    },
                    VkImageSubresourceRange{
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        0,
                        VK_REMAINING_MIP_LEVELS,
                        0,
                        VK_REMAINING_ARRAY_LAYERS
                    }
                };
                if (const VkResult imageViewCreationResult = vkCreateImageView(
                        logicalDevice, &imageViewCreateInfo, nullptr, &view);
                    imageViewCreationResult != VK_SUCCESS) {
                    // TODO -> log!
                    return false;
                }
                swapchainImageViewIndices[swapchainImageUids[image]] = swapchainImageViews.size() - 1;
            }
            return true;
        }

        VkExtent2D getMaximumImageExtent() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface capabilities!");
            }

            return surfaceCapabilities.maxImageExtent;
        }

        VkExtent2D getMinimumImageExtent() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface capabilities!");
            }

            return surfaceCapabilities.minImageExtent;
        }

        [[nodiscard]] bool inputExtentIsValid(VkExtent2D extent2D) const {
            return (extent2D.width <= maximumSwapchainImageExtent.width
                    && extent2D.height <= maximumSwapchainImageExtent.height)
                   && (extent2D.width >= minimumSwapchainImageExtent.width &&
                       extent2D.height >= minimumSwapchainImageExtent.height);
        }
    };
}
