//
// Created by paull on 2022-06-19.
//

#include "VulkanLogicalDevice.hpp"

#include <memory>
#include <stdexcept>
#include <algorithm>

namespace pEngine::girEngine::backend::appContext::vulkan {
    const std::unordered_map<VulkanLogicalDevice::SupportedDeviceLayer, std::string>
    VulkanLogicalDevice::DEVICE_LAYER_NAME_CONVERSION_MAP = {
        // TODO - add in the conversions to name strings that will be fed to vulkan
    };


    const std::unordered_map<VulkanLogicalDevice::SupportedDeviceExtension, std::string>
    VulkanLogicalDevice::DEVICE_EXTENSION_NAME_CONVERSION_MAP = {
        // TODO - add in the conversions to name strings that will be fed to vulkan
        {SupportedDeviceExtension::SWAPCHAIN_EXTENSION, VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        {SupportedDeviceExtension::SYNC_2, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME},
        {SupportedDeviceExtension::DYNAMIC_RENDERING, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME},
        {SupportedDeviceExtension::NONSEMANTIC_SHADER_INFO, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME},
    };

    VulkanLogicalDevice::VulkanLogicalDevice(const CreationInput &creationInput) {
        if (!isValidVulkanLogicalDeviceCreateInfo(creationInput)) {
            throw std::runtime_error("Unable to create VulkanLogicalDevice because of invalid input data!");
        }

        physicalDevice = creationInput.physicalDevice;

        std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = generateDeviceQueueCreateInfos(
            creationInput.physicalDevice);


        std::vector<const char *> enabledDeviceLayers = getEnabledDeviceLayers(creationInput);
        std::vector<const char *> enabledDeviceExtensions = getEnabledDeviceExtensions(creationInput);
        VkDeviceCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(deviceQueueCreateInfos.size()),
            deviceQueueCreateInfos.data(),
            static_cast<uint32_t>(enabledDeviceLayers.size()),
            enabledDeviceLayers.data(),
            static_cast<uint32_t>(enabledDeviceExtensions.size()),
            enabledDeviceExtensions.data()
        };

        // SYNC 2: have to use this
        VkPhysicalDeviceSynchronization2Features synchronization2Features{};
        if (std::ranges::any_of(enabledDeviceExtensions, [&](const char *extensionName) {
            return std::string(extensionName) == std::string(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        })) {
            synchronization2Features = {
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
                nullptr,
                VK_TRUE
            };
            createInfo.pNext = &synchronization2Features;
        }

        // DYNAMIC RENDERING: have to use this too
        VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatureEnable{};
        if (std::ranges::any_of(enabledDeviceExtensions, [&](const char *extensionName) {
            return std::string(extensionName) == std::string(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        })) {
            dynamicRenderingFeatureEnable = {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
                .pNext = (synchronization2Features.sType) ? &synchronization2Features : nullptr,
                .dynamicRendering = VK_TRUE,
                // add existing stuff to pNext chain if it's present
            };
            createInfo.pNext = &dynamicRenderingFeatureEnable;
        }

        auto result = vkCreateDevice(creationInput.physicalDevice, &createInfo, nullptr, &logicalDevice);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error in VulkanLogicalDevice() - Unable to create logical device!");
        }

        getGraphicsQueueHandle();

        getComputeQueueHandle();
    }

    void VulkanLogicalDevice::getComputeQueueHandle() {
        vkGetDeviceQueue(this->logicalDevice, this->computeQueueFamilyIndex, 0, &this->computeQueue);
        if (this->computeQueue == VK_NULL_HANDLE) {
            throw std::runtime_error(
                "Error in VulkanLogicalDevice::getComputeQueueHandle() - Unable to acquire compute queue from created device!");
        }
    }

    void VulkanLogicalDevice::getGraphicsQueueHandle() {
        vkGetDeviceQueue(logicalDevice, graphicsQueueFamilyIndex, 0, &graphicsQueue);
        if (graphicsQueue == VK_NULL_HANDLE) {
            throw std::runtime_error(
                "Error in VulkanLogicalDevice::getGraphicsQueueHandle() - Unable to acquire graphics queue from created device!");
        }
    }

    bool VulkanLogicalDevice::isValidVulkanLogicalDeviceCreateInfo(
        const VulkanLogicalDevice::CreationInput &info) {
        return (
            info.physicalDevice != VK_NULL_HANDLE
            && !info.name.empty()
        );
    }

    std::vector<VkDeviceQueueCreateInfo> VulkanLogicalDevice::generateDeviceQueueCreateInfos(
        VkPhysicalDevice physicalDevice) {
        std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties =
                getPhysicalDeviceQueueFamilies(physicalDevice);

        unsigned int graphicsQueueFamilyIndex = getGraphicsQueueFamilyIndex(physicalDeviceQueueFamilyProperties);
        unsigned int computeQueueFamilyIndex = getComputeQueueFamilyIndex(physicalDeviceQueueFamilyProperties);

        VkDeviceQueueCreateInfo graphicsDeviceQueueCreateInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, graphicsQueueFamilyIndex, 1,
            &ARBITRARY_QUEUE_PRIORITY
        };

        VkDeviceQueueCreateInfo computeDeviceQueueCreateInfo =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, computeQueueFamilyIndex, 1,
            &ARBITRARY_QUEUE_PRIORITY
        };

        return {graphicsDeviceQueueCreateInfo, computeDeviceQueueCreateInfo};
    }

    std::vector<const char *> VulkanLogicalDevice::getEnabledDeviceLayers(const CreationInput &input) {
        std::vector<const char *> enabledDeviceLayerNameStrings = {};
        for (const auto &enabledDeviceLayer: input.enabledDeviceLayers) {
            enabledDeviceLayerNameStrings.push_back(DEVICE_LAYER_NAME_CONVERSION_MAP.at(enabledDeviceLayer).c_str());
        }
        return enabledDeviceLayerNameStrings;
    }

    std::vector<const char *> VulkanLogicalDevice::getEnabledDeviceExtensions(const CreationInput &input) {
        std::vector<const char *> enabledDeviceExtensionNameStrings = {};
        for (const auto &enabledDeviceExtension: input.enabledDeviceExtensions) {
            enabledDeviceExtensionNameStrings.push_back(
                DEVICE_EXTENSION_NAME_CONVERSION_MAP.at(enabledDeviceExtension).c_str());
        }
        return enabledDeviceExtensionNameStrings;
    }

    std::vector<VkQueueFamilyProperties> VulkanLogicalDevice::getPhysicalDeviceQueueFamilies(
        VkPhysicalDevice physicalDevice) {
        uint32_t numberOfPhysicalDeviceQueues = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numberOfPhysicalDeviceQueues, nullptr);

        std::vector<VkQueueFamilyProperties> physicalDeviceQueueFamilyProperties(numberOfPhysicalDeviceQueues);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                 &numberOfPhysicalDeviceQueues,
                                                 physicalDeviceQueueFamilyProperties.data());

        return physicalDeviceQueueFamilyProperties;
    }

    unsigned int VulkanLogicalDevice::getComputeQueueFamilyIndex(
        std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies) {
        unsigned int computeQueueFamilyIndex = 0;
        for (const auto &queueProperties: physicalDeviceQueueFamilies) {
            if (queueProperties.queueFlags & VK_QUEUE_COMPUTE_BIT &&
                !(queueProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                return computeQueueFamilyIndex;
            }
            computeQueueFamilyIndex++;
        }

        throw std::runtime_error("No compute queue available for chosen physical device!");
    }

    unsigned int VulkanLogicalDevice::getGraphicsQueueFamilyIndex(
        std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies) {
        unsigned int graphicsQueueFamilyIndex = 0;
        for (const auto &queueProperties: physicalDeviceQueueFamilies) {
            if (queueProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                return graphicsQueueFamilyIndex;
            }
            graphicsQueueFamilyIndex++;
        }

        throw std::runtime_error("No graphics queue available for chosen physical device!");
    }

    const uint32_t &VulkanLogicalDevice::getGraphicsQueueFamilyIndex() const {
        return graphicsQueueFamilyIndex;
    }

    const uint32_t &VulkanLogicalDevice::getComputeQueueFamilyIndex() const {
        return computeQueueFamilyIndex;
    }
} // namespace PGraphics
