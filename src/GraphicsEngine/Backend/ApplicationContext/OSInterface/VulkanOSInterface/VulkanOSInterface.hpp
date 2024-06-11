//
// Created by paull on 2022-06-28.
//

#pragma once

#include <vulkan/vulkan.h>

#ifdef _WIN32

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#endif

#include <memory>
#include <vector>
#include <stdexcept>

#include "../OSInterface.hpp"
#include "../OSWindow/OSWindow.h"
#include "VulkanSwapchain/VulkanSwapchain.hpp"
#include "../OSWindow/Win32Window/Win32Window.hpp"

namespace pEngine::girEngine::backend::appContext::osInterface::vulkan {

    class VulkanOSInterface : public OSInterface {
    public:
        struct CreationInput : public OSInterface::CreateInfo {

            VkInstance instance{};
            VkDevice logicalDevice{};
            VkPhysicalDevice physicalDevice{};

            VkCompositeAlphaFlagBitsKHR compositeAlphaFlag;
            VkPresentModeKHR swapchainPresentMode;

            unsigned numberOfSwapchainImages = 3;
            VkFormat swapchainImageFormat;
            VkColorSpaceKHR swapchainImageColorSpace;
            VkImageUsageFlags swapchainImageUsages{};
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform;
            bool swapchainShouldClipObscuredTriangles = true;
            int initialWidth;
            int initialHeight;
        };

        explicit VulkanOSInterface(const CreationInput &creationInput);

        ~VulkanOSInterface();

        [[nodiscard]] VkRect2D getRenderArea() const;

        [[nodiscard]] OSInterface::WindowPlatform getWindowPlatform() const override {
            return windowPlatform;
        }

        CreateWindowResult createWindow() override;

        ShowWindowResult showWindow() override;

        ResizeWindowResult resizeWindow(unsigned int length, unsigned int width) override;

        std::shared_ptr<swapchain::vulkan::VulkanSwapchain> &getSwapchain() {
            return vulkanSwapchain;
        }

        std::shared_ptr<osWindow::win32::Win32Window> getWin32Window() {
            return std::dynamic_pointer_cast<osWindow::win32::Win32Window>(window);
        }

        [[nodiscard]] const VkViewport &getViewport() const {
            return viewport;
        }

        [[nodiscard]] const VkRect2D &getScissor() const;

    private:
        WindowPlatform windowPlatform;

        VkInstance instance;
        VkDevice logicalDevice;
        VkPhysicalDevice physicalDevice;

        std::shared_ptr<osWindow::OSWindow> window;

        /**
         * In Vulkan, a VkSurfaceKHR is what will accept rendering data from the core Vulkan runtime and allow an
         * operating system to present it.
         *
         * I think it makes sense to keep this in the vulkan os interface itself, since it's kinda its own thing
         * but doesn't really make sense to split off as another class
         */
        VkSurfaceKHR surface;

        std::shared_ptr<osInterface::swapchain::vulkan::VulkanSwapchain> vulkanSwapchain = nullptr;

        // hardcoding a single viewport & scissor per app for now
        VkViewport viewport;
        VkRect2D scissor;

        // TODO - refactor this so we can easily resize the window and recreate the swapchain/acquire images when we do

        [[nodiscard]] std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats() const {
            uint32_t numberOfSurfaceFormats = 0;
            auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numberOfSurfaceFormats,
                                                               nullptr);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface formats!");
            }

            std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats(numberOfSurfaceFormats);
            result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                                          surface,
                                                          &numberOfSurfaceFormats,
                                                          physicalDeviceSurfaceFormats.data());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface formats!");
            }
            return physicalDeviceSurfaceFormats;
        }

        // TODO - really re-evaluate this one; we can probably implement a simple user-specification thing
        // where we convert b/w our platform-agnostic texel format enum into the vulkan one, and then we
        // basically just look for it in the array of physical device surface formats that we acquire
        VkFormat getSwapchainImageFormat() {
            std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats = getPhysicalDeviceSurfaceFormats();

            // default behavior here: choose the first VkSurfaceFormatKHR that's available (should change this)
            return physicalDeviceSurfaceFormats.front().format;
        }

        // TODO - similar to the image format one, we have to re-evaluate how this works (make it user-specifiable)
        VkColorSpaceKHR getSwapchainImageColorSpace() {
            std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats = getPhysicalDeviceSurfaceFormats();

            // default behavior here: choose the first VkSurfaceFormatKHR that's available
            return physicalDeviceSurfaceFormats.front().colorSpace;
        }

        VkExtent2D getMinimumSwapchainImageExtent() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface capabilities!");
            }

            return surfaceCapabilities.minImageExtent;
        }

        VkExtent2D getMaximumSwapchainImageExtent() {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
            auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface capabilities!");
            }

            return surfaceCapabilities.maxImageExtent;
        }

        void createSwapchain(const CreationInput &input) {
            vulkanSwapchain = std::make_shared<swapchain::vulkan::VulkanSwapchain>(
                    swapchain::vulkan::VulkanSwapchain::CreationInput{
                            input.instance,
                            input.logicalDevice,
                            input.physicalDevice,
                            input.swapchainPresentMode,
                            input.compositeAlphaFlag,
                            input.numberOfSwapchainImages,
                            {window->getWindowWidth(),
                             window->getWindowHeight()}, // try making extent based off window size?
                            input.swapchainImageFormat,
                            input.swapchainImageColorSpace,
                            input.swapchainImageUsages,
                            surface,
                            input.swapchainImagePreTransform,
                            input.swapchainShouldClipObscuredTriangles,
                            boost::none // TODO - support old swapchain usage lol
                    });
        }

        void createWin32WindowAndSurface(int initialWidth, int initialHeight);

        void build(const CreationInput &input);

    };

}// namespace PG