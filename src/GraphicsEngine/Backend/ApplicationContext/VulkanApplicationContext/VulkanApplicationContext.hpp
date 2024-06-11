//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>

#include "../ApplicationContext.hpp"
#include "../OSInterface/VulkanOSInterface/VulkanOSInterface.hpp"
#include "../../VulkanBackend/VulkanInstance/VulkanInstance.hpp"
#include "../../VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
#include "../../VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.hpp"
#include "../../VulkanBackend/VulkanDearImguiContext/VulkanDearImguiContext.hpp"

namespace pEngine::girEngine::backend::appContext::vulkan {
    class VulkanApplicationContext : public ApplicationContext {
    public:
        struct CreationInput {
            std::string vulkanAppName;
            std::string vulkanEngineName;

            std::vector<VulkanInstance::SupportedInstanceExtension> enabledInstanceExtensions;

            std::vector<VulkanInstance::SupportedLayers> enabledInstanceLayers;

            std::vector<VulkanLogicalDevice::SupportedDeviceExtension> enabledDeviceExtensions;

            std::vector<VulkanLogicalDevice::SupportedDeviceLayer> enabledDeviceLayers;

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

        static const char *DEFAULT_NAME;

        explicit VulkanApplicationContext(const CreationInput &creationInput);

        ~VulkanApplicationContext() override = default;

        std::shared_ptr<osInterface::vulkan::VulkanOSInterface> getOSInterface() {
            return osInterface;
        }

        const std::shared_ptr<VulkanLogicalDevice> &getLogicalDevice() {
            return logicalDevice;
        }

        VkPhysicalDevice getPhysicalDevice() {
            return physicalDevice->getPhysicalDevice();
        }

        VkInstance getInstance() {
            return vulkanInstance->getVkInstance();
        }

        // TODO - figure out where this render area rect should be living

        VkQueue getGraphicsQueue() {
            return logicalDevice->getGraphicsQueue();
        }

        [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const {
            return logicalDevice->getGraphicsQueueFamilyIndex();
        }

        [[nodiscard]] uint32_t getComputeQueueFamilyIndex() const {
            return logicalDevice->getComputeQueueFamilyIndex();
        }

    private:
        std::shared_ptr<VulkanInstance> vulkanInstance;
        std::shared_ptr<VulkanPhysicalDevice> physicalDevice;
        std::shared_ptr<VulkanLogicalDevice> logicalDevice;

        std::shared_ptr<osInterface::vulkan::VulkanOSInterface> osInterface = nullptr;

        std::shared_ptr<VulkanDearImguiContext> dearImguiContext = nullptr;

        void initializeDearImguiContext(const CreationInput &input);

        static VkFormat obtainSwapchainImageFormat(VkFormat desiredFormat);
    };
} // namespace PGraphics
