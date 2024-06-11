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
#include <stdexcept>

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::appContext::vulkan {
    enum class PhysicalDeviceChoice {
        BEST_GPU,
        ANY
    };

    class VulkanPhysicalDevice {
    public:
        struct CreationInput {
            /**
             * TODO - add a lot more configurability here for choosing GPU and what not;
             * for now, we'll just design it to just look for the "best" GPU (ie the best one on your system)
             */
            const util::UniqueIdentifier uid;

            const std::string name;

            const VkInstance &instance;

            const PhysicalDeviceChoice choice = PhysicalDeviceChoice::BEST_GPU;

            VkPhysicalDeviceType desiredPhysicalDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

            uint32_t desiredApiVersion;
        };

        explicit VulkanPhysicalDevice(const CreationInput &creationInput) {
            if (!isValidVulkanPhysicalDeviceCreateInfo(creationInput)) {
                throw std::runtime_error("Unable to create VulkanPhysicalDevice because of invalid creation input!");
            }

            std::vector<VkPhysicalDevice> availablePhysicalDevices = enumeratePhysicalDevices(creationInput.instance);
            if (availablePhysicalDevices.empty()) {
                // TODO - logging / actual error handling
                throw std::runtime_error("No physical devices are available!");
            }

            if (availablePhysicalDevices.size() == 1) {
                // todo when logging is implemented: give warning
            }

            VkPhysicalDevice chosenPhysicalDevice = choosePhysicalDevice(availablePhysicalDevices, creationInput);
            if (chosenPhysicalDevice == nullptr) {
                throw std::runtime_error("Unable to choose a physical device!");
            }

            physicalDevice = chosenPhysicalDevice;
        }

        ~VulkanPhysicalDevice() = default;


        const VkPhysicalDevice &getPhysicalDevice() {
            return physicalDevice;
        }

    private:
        VkPhysicalDeviceType desiredPhysicalDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        static bool isValidVulkanPhysicalDeviceCreateInfo(const CreationInput &info);

        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(const VkInstance &instance);

        static VkPhysicalDevice choosePhysicalDevice(const std::vector<VkPhysicalDevice> &availablePhysicalDevices,
                                                     const CreationInput &info);

        static bool isNewBestPhysicalDeviceChoice(VkPhysicalDeviceLimits deviceToCheck,
                                                  VkPhysicalDeviceLimits currentBestDevice);
    };
} // namespace PGraphics
