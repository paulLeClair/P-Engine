//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>

#include "../../VulkanBackend/VulkanInstance/VulkanInstance.hpp"
#include "../../VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
#include "../../VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.hpp"

#define VK_LAYER_PRINTF_TO_STDOUT 1

struct ImGuiContext;

namespace pEngine::girEngine::backend::vulkan {
    class VulkanOSInterface;

    class VulkanApplicationContext {
    public:
        struct CreationInput {
            std::string vulkanAppName;
            std::string vulkanEngineName;

            std::vector<vulkan::VulkanInstance::SupportedInstanceExtension> enabledInstanceExtensions;

            std::vector<vulkan::VulkanInstance::SupportedLayers> enabledInstanceLayers;

            std::vector<vulkan::VulkanLogicalDevice::SupportedDeviceExtension> enabledDeviceExtensions;

            std::vector<vulkan::VulkanLogicalDevice::SupportedDeviceLayer> enabledDeviceLayers;

            // TODO - expand this eventually to support multiple physical/logical devices
            std::string physicalDeviceLabel;
            std::string logicalDeviceLabel;

            // I guess we'll have to pass in the vulkan version from here, it's been set at 0
            uint32_t appVersion = VK_MAKE_VERSION(0, 1, 0);
            uint32_t engineVersion = VK_MAKE_VERSION(0, 1, 0);
            uint32_t vulkanApiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

            // swapchain configuration
            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            unsigned numberOfSwapchainImages = 3; // triple buffer by default
            VkFormat desiredSwapchainImageFormat;
            VkColorSpaceKHR swapchainImageColorSpace;
            VkImageUsageFlags swapchainImageUsages{};
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform;
            bool swapchainShouldClipObscuredTriangles = true;
            int initialWindowWidth;
            int initialWindowHeight;
        };

        explicit VulkanApplicationContext(const CreationInput &creationInput);

        ~VulkanApplicationContext() = default;

        std::shared_ptr<vulkan::VulkanOSInterface> getOSInterface() {
            return osInterface;
        }

        const std::shared_ptr<VulkanLogicalDevice> &getLogicalDevice() {
            return logicalDevice;
        }

        [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const {
            return physicalDevice->getPhysicalDevice();
        }

        [[nodiscard]] VkInstance getInstance() const {
            return vulkanInstance->getVkInstance();
        }

        // TODO - figure out where this render area rect should be living

        [[nodiscard]] VkQueue getGraphicsQueue() const {
            return logicalDevice->getGraphicsQueue();
        }

        [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const {
            return logicalDevice->getGraphicsQueueFamilyIndex();
        }

        [[nodiscard]] uint32_t getComputeQueueFamilyIndex() const {
            return logicalDevice->getComputeQueueFamilyIndex();
        }

    private:
        std::shared_ptr<vulkan::VulkanInstance> vulkanInstance;
        std::shared_ptr<vulkan::VulkanPhysicalDevice> physicalDevice;
        std::shared_ptr<vulkan::VulkanLogicalDevice> logicalDevice;

        std::shared_ptr<vulkan::VulkanOSInterface> osInterface = nullptr;
        ;

        ImGuiContext *imGuiContext;

        static VkFormat obtainSwapchainImageFormat(VkFormat desiredFormat);
    };
} // namespace PGraphics
