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

namespace pEngine::girEngine::backend::appContext::vulkan {
    class VulkanLogicalDevice {
    public:
        enum class SupportedDeviceLayer {
            UNKNOWN
        };

        enum class SupportedDeviceExtension {
            UNKNOWN,
            SWAPCHAIN_EXTENSION,
            SYNC_2,
            DYNAMIC_RENDERING,
            NONSEMANTIC_SHADER_INFO,
        };

        struct CreationInput {
            // all info required to create a VkDevice, in high-level format
            std::string name;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

            std::vector<SupportedDeviceExtension> enabledDeviceExtensions = {};
            std::vector<SupportedDeviceLayer> enabledDeviceLayers = {};
        };

        explicit VulkanLogicalDevice(const CreationInput &creationInput);

        VulkanLogicalDevice() = default;

        ~VulkanLogicalDevice() {
            vkDestroyDevice(logicalDevice, nullptr);
        }

        [[nodiscard]] const VkDevice &getVkDevice() const {
            return logicalDevice;
        }

        /* Device Queues Interface */
        [[nodiscard]] const VkQueue &getGraphicsQueue() const {
            return graphicsQueue;
        }

        [[nodiscard]] const VkQueue &getComputeQueue() const {
            return computeQueue;
        }

        [[nodiscard]] const uint32_t &getGraphicsQueueFamilyIndex() const;

        [[nodiscard]] const uint32_t &getComputeQueueFamilyIndex() const;

        uint32_t getMinimumUniformBufferAlignment() const {
            VkPhysicalDeviceProperties properties = {};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            return properties.limits.minUniformBufferOffsetAlignment;
        }

    private:
        constexpr static float ARBITRARY_QUEUE_PRIORITY = 0.5;

        static const std::unordered_map<SupportedDeviceLayer, std::string> DEVICE_LAYER_NAME_CONVERSION_MAP;

        static const std::unordered_map<SupportedDeviceExtension, std::string> DEVICE_EXTENSION_NAME_CONVERSION_MAP;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;

        uint32_t graphicsQueueFamilyIndex = 0;
        uint32_t computeQueueFamilyIndex = 0;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;

        static bool isValidVulkanLogicalDeviceCreateInfo(const CreationInput &info);

        static std::vector<VkDeviceQueueCreateInfo> generateDeviceQueueCreateInfos(VkPhysicalDevice physicalDevice);

        static std::vector<const char *> getEnabledDeviceLayers(const CreationInput &input);

        static std::vector<const char *> getEnabledDeviceExtensions(const CreationInput &input);

        static std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice);

        static unsigned int
        getGraphicsQueueFamilyIndex(std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies);

        static unsigned int
        getComputeQueueFamilyIndex(std::vector<VkQueueFamilyProperties> &physicalDeviceQueueFamilies);

        void getGraphicsQueueHandle();

        void getComputeQueueHandle();
    };
} // namespace PGraphics
