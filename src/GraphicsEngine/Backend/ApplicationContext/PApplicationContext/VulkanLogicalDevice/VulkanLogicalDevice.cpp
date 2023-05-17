//
// Created by paull on 2022-06-19.
//

#include "VulkanLogicalDevice.hpp"

#include <memory>
#include <stdexcept>

namespace PGraphics {

    VulkanLogicalDevice::VulkanLogicalDevice(const VulkanLogicalDevice::CreationInput &creationInput) {
        if (!isValidVulkanLogicalDeviceCreateInfo(creationInput)) {
            throw std::runtime_error("Unable to create VulkanLogicalDevice because of invalid input data!");
        }


        std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = generateDeviceQueueCreateInfos(
                creationInput.physicalDevice);


        std::vector<const char *> enabledDeviceLayers = getEnabledDeviceLayers();
        std::vector<const char *> enabledDeviceExtensions = getEnabledDeviceExtensions();
        VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                         NULL,
                                         0,
                                         static_cast<uint32_t>( deviceQueueCreateInfos.size()),
                                         deviceQueueCreateInfos.data(),
                                         0,
                                         nullptr,
                                         static_cast<uint32_t>( enabledDeviceExtensions.size()),
                                         enabledDeviceExtensions.data()};

        if (vkCreateDevice(creationInput.physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
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
                {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL, 0, graphicsQueueFamilyIndex, 1,
                 &ARBITRARY_QUEUE_PRIORITY};

        VkDeviceQueueCreateInfo computeDeviceQueueCreateInfo =
                {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, NULL, 0, computeQueueFamilyIndex, 1,
                 &ARBITRARY_QUEUE_PRIORITY};

        return {graphicsDeviceQueueCreateInfo, computeDeviceQueueCreateInfo};
    }

    std::vector<const char *> VulkanLogicalDevice::getEnabledDeviceLayers() {
        return {
                // nothing - we don't use any device layers (for now?)
        };
    }

    std::vector<const char *> VulkanLogicalDevice::getEnabledDeviceExtensions() {
        return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
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

    uint32_t VulkanLogicalDevice::getGraphicsQueueFamilyIndex() const {
        return graphicsQueueFamilyIndex;
    }

    uint32_t VulkanLogicalDevice::getComputeQueueFamilyIndex() const {
        return computeQueueFamilyIndex;
    }


}// namespace PGraphics