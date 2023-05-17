//
// Created by paull on 2022-06-19.
//

#include "VulkanPhysicalDevice.hpp"

#include <vector>

namespace PGraphics {

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
            const std::vector<VkPhysicalDevice> &availablePhysicalDevices,
            const VulkanPhysicalDevice::CreationInput &info) {
        // TODO - support other ways of selecting the physical device (maybe even allow supplying a callback?)
        if (info.choice != CreationInput::PhysicalDeviceChoice::BEST_GPU) {
            return nullptr;
        }

        VkPhysicalDevice bestDeviceChoice = nullptr;
        VkPhysicalDeviceProperties bestDeviceChoiceProperties = {};
        VkPhysicalDeviceProperties deviceProperties = {};
        for (const auto &physicalDevice: availablePhysicalDevices) {
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

            if (deviceProperties.apiVersion < VK_VERSION_1_3) {
                continue;
            }

            // TODO - make the type of device specifiable
            if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                continue;
            }

            if (isNewBestPhysicalDeviceChoice(deviceProperties.limits, bestDeviceChoiceProperties.limits)) {
                bestDeviceChoice = physicalDevice;
                bestDeviceChoiceProperties = deviceProperties;
                continue;
            }
        }

        return bestDeviceChoice;
    }

    bool VulkanPhysicalDevice::isNewBestPhysicalDeviceChoice(VkPhysicalDeviceLimits deviceToCheck,
                                                             VkPhysicalDeviceLimits currentBestDevice) {
        // TODO - figure out a good way to compare these two limits to figure out which device is better...
        if (deviceToCheck.maxBoundDescriptorSets < currentBestDevice.maxBoundDescriptorSets) {
            return false;
        }

        if (deviceToCheck.maxColorAttachments < currentBestDevice.maxColorAttachments) {
            return false;
        }

        if (deviceToCheck.maxDrawIndexedIndexValue < currentBestDevice.maxDrawIndexedIndexValue) {
            return false;
        }

        if (deviceToCheck.maxFramebufferHeight < currentBestDevice.maxFramebufferHeight ||
            deviceToCheck.maxFramebufferWidth < currentBestDevice.maxFramebufferWidth) {
            return false;
        }

        if (deviceToCheck.maxUniformBufferRange < currentBestDevice.maxUniformBufferRange) {
            return false;
        }

        /* TODO - make this check physical device queues, or break that off into a separate evaluation function */

        return true;
    }


}// namespace PGraphics