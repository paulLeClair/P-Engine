//
// Created by paull on 2022-06-28.
//

#pragma once

#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <memory>
#include <vector>

#include "../../PApplicationContext/PApplicationContext.hpp"
#include "../WindowSystem.hpp"
#include "../OSWindow/OSWindow.h"

namespace PGraphics {

    class PApplicationContext;

    class PWindowSystem : public WindowSystem {
    public:
        struct CreationInput : public WindowSystem::CreateInfo {

            // we'll have to include a lot of the configuration info for the swapchain
            VkInstance instance;
            VkDevice logicalDevice;
            VkPhysicalDevice physicalDevice;

            VkCompositeAlphaFlagBitsKHR compositeAlphaFlag;
            VkPresentModeKHR swapchainPresentMode;

//            VkExtent2D swapchainImageExtent;

            CreationInput(WindowPlatform windowPlatform, VkInstance instance, VkDevice logicalDevice,
                          VkPhysicalDevice physicalDevice, VkCompositeAlphaFlagBitsKHR compositeAlphaFlag,
                          VkPresentModeKHR swapchainPresentMode)
//                          , VkExtent2D imageExtent)
                    : instance(
                    instance),
                      logicalDevice(
                              logicalDevice),
                      physicalDevice(
                              physicalDevice),
                      compositeAlphaFlag(
                              compositeAlphaFlag),
                      swapchainPresentMode(
                              swapchainPresentMode),
//                      swapchainImageExtent(
//                              imageExtent),
                      WindowSystem::CreateInfo(
                              windowPlatform) {

            }
        };

        explicit PWindowSystem(const CreationInput &creationInput);

        ~PWindowSystem();

        [[nodiscard]] VkRect2D getRenderArea() const;

        [[nodiscard]] WindowSystem::WindowPlatform getWindowPlatform() const override {
            return windowPlatform;
        }

        VkImage getSwapchainImage(unsigned int frameIndex);

        [[nodiscard]] const uint32_t &getSwapchainImageWidth() const {
            return swapchainImageExtent.width;
        }

        [[nodiscard]] const uint32_t &getSwapchainImageHeight() const {
            return swapchainImageExtent.height;
        }

        [[nodiscard]] VkSwapchainKHR getSwapchain() const {
            return swapchain;
        }

        CreateWindowResult createWindow() override;

        ShowWindowResult showWindow() override;

        ResizeWindowResult resizeWindow(unsigned int length, unsigned int width) override;

    private:
        WindowPlatform windowPlatform;

        VkInstance instance;
        VkDevice logicalDevice;
        VkPhysicalDevice physicalDevice;

        std::shared_ptr<OSWindow> osWindow;

        /**
         * In Vulkan, a VkSurfaceKHR is what will accept rendering data from the core Vulkan runtime and allow an
         * operating system to present it
         */
        VkSurfaceKHR surface;

        /**
         * The swapchain is another window-system-facing Vulkan thing
         */
        VkSwapchainKHR swapchain;
        VkPresentModeKHR swapchainPresentMode;
        VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag;

        std::vector<VkImage> swapchainImages = {};

        VkExtent2D swapchainImageExtent;

        // TODO - refactor this so we can easily resize the window and recreate the swapchain/acquire images when we do

        enum class CreateSwapchainResult {
            SUCCESS,
            FAILURE
        };

        PWindowSystem::CreateSwapchainResult createSwapchain();

        enum class AcquireSwapchainImagesResult {
            SUCCESS,
            FAILURE
        };

        AcquireSwapchainImagesResult acquireSwapchainImages();

        void createWin32WindowAndSurface();

        VkImageUsageFlags getSwapchainImageUsages();

        VkSurfaceTransformFlagBitsKHR getSwapchainPreTransform();

        VkExtent2D getMinimumSwapchainImageExtent();

        VkExtent2D getMaximumSwapchainImageExtent();

        void setSwapchainImageExtent(VkExtent2D newSwapchainImageExtent) {
            const VkExtent2D &maximumSwapchainImageExtent = getMaximumSwapchainImageExtent();
            if (newSwapchainImageExtent.width > maximumSwapchainImageExtent.width ||
                newSwapchainImageExtent.height > maximumSwapchainImageExtent.height) {
                // TODO - log
                throw std::runtime_error(
                        "Error in setSwapchainImageExtent - new swapchain image extent is beyond the maximum!");
            }

            const VkExtent2D &minimumSwapchainImageExtent = getMinimumSwapchainImageExtent();
            if (newSwapchainImageExtent.width < minimumSwapchainImageExtent.width ||
                newSwapchainImageExtent.height < minimumSwapchainImageExtent.height) {
                // TODO - log
                throw std::runtime_error(
                        "Error in setSwapchainImageExtent - new swapchain image extent is less than the minimum!");
            }

            swapchainImageExtent = newSwapchainImageExtent;
        }

        VkFormat getSwapchainImageFormat();

        VkColorSpaceKHR
        getSwapchainImageColorSpace();

        [[nodiscard]] std::vector<VkSurfaceFormatKHR>
        getPhysicalDeviceSurfaceFormats() const;

        void build();

        void initializeSwapchainImageExtent();
    };

}// namespace PG