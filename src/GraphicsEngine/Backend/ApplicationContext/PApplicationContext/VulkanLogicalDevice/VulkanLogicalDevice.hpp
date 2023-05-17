//
// Created by paull on 2022-06-19.
//

#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace PGraphics {

    class VulkanLogicalDevice {
    public:
        struct CreationInput {
            // all info required to create a VkDevice, in high-level format
            std::string name;
            VkPhysicalDevice physicalDevice;

            CreationInput(std::string name, VkPhysicalDevice physicalDevice)
                    : name(std::move(name)),
                      physicalDevice(physicalDevice) {
            }
        };

        explicit VulkanLogicalDevice(const CreationInput &creationInput);

        ~VulkanLogicalDevice() {
            vkDestroyDevice(logicalDevice, nullptr);
        }

        VkDevice getLogicalDevice() {
            return logicalDevice;
        }

        /* Device Queues Interface */
        VkQueue getGraphicsQueue() {
            return graphicsQueue;
        }

        VkQueue getComputeQueue() {
            return computeQueue;
        }

        [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const;

        [[nodiscard]] uint32_t getComputeQueueFamilyIndex() const;

    private:
        constexpr static const float ARBITRARY_QUEUE_PRIORITY = 0.5;

        VkDevice logicalDevice = VK_NULL_HANDLE;

        uint32_t graphicsQueueFamilyIndex = 0;
        uint32_t computeQueueFamilyIndex = 0;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;

        static bool isValidVulkanLogicalDeviceCreateInfo(const CreationInput &info);

        static std::vector<VkDeviceQueueCreateInfo> generateDeviceQueueCreateInfos(VkPhysicalDevice physicalDevice);

        static std::vector<const char *> getEnabledDeviceLayers();

        static std::vector<const char *> getEnabledDeviceExtensions();

        static std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice);

        static unsigned int
        getGraphicsQueueFamilyIndex(std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies);

        static unsigned int
        getComputeQueueFamilyIndex(std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies);

        void getGraphicsQueueHandle();

        void getComputeQueueHandle();
    };

}// namespace PGraphics
