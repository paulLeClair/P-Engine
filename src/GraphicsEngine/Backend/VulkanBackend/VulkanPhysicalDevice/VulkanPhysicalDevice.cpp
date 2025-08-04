//
// Created by paull on 2022-06-19.
//

#include "VulkanPhysicalDevice.hpp"

#include <vector>

namespace pEngine::girEngine::backend::appContext::vulkan {
    bool VulkanPhysicalDevice::isValidVulkanPhysicalDeviceCreateInfo(
        const VulkanPhysicalDevice::CreationInput &info) {
        return !(info.name.empty() || info.instance == nullptr);
    }


    std::vector<VkPhysicalDevice> VulkanPhysicalDevice::enumeratePhysicalDevices(const VkInstance &instance) {
        unsigned int numPhysicalDevices;
        if (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, nullptr) != VK_SUCCESS) {
            return {};
        }

        std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
        if (vkEnumeratePhysicalDevices(instance, &numPhysicalDevices, physicalDevices.data()) != VK_SUCCESS) {
            return {};
        }
        return physicalDevices;
    }

    VkPhysicalDevice VulkanPhysicalDevice::choosePhysicalDevice(
        const std::vector<VkPhysicalDevice> &availablePhysicalDevices) {
        // TODO - support other ways of selecting the physical device

        if (availablePhysicalDevices.empty()) {
            throw std::runtime_error("Error in VulkanPhysicalDevice::choosePhysicalDevice -> No available GPUs found");
        }

        if (availablePhysicalDevices.size() > 1) {
            for (const auto &physicalDevice: availablePhysicalDevices) {
                // TODO -> implement adequate handling of physical devices
            }
        }

        return availablePhysicalDevices[0];
    }

    bool VulkanPhysicalDevice::isNewBestPhysicalDeviceChoice(VkPhysicalDeviceLimits deviceToCheck,
                                                             VkPhysicalDeviceLimits currentBestDevice) {
        // TODO - figure out a good way to compare these two limits to figure out which device is better...
        /* TODO - make this check physical device queues, or break that off into a separate evaluation function */
        return true;
    }
} // namespace PGraphics
