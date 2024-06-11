//
// Created by paull on 2024-05-12.
//

#pragma once

#include <boost/optional.hpp>
#include <vulkan/vulkan.hpp>

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
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform = VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

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
                    VK_SHARING_MODE_EXCLUSIVE,// TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                    0,// TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                    nullptr,// TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
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

        VkImage getSwapchainImage(unsigned imageIndex) {
            // TODO - re-evaluate how this works; however, to keep it agnostic w.r.t present mode, an index-based access seems ok
            return swapchainImages[imageIndex];
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


        std::vector<VkImage> swapchainImages = {};

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
            return vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, swapchainImages.data())
                   == VK_SUCCESS;
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
