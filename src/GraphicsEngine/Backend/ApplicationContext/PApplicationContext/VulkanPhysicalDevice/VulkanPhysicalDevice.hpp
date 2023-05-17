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

namespace PGraphics {

    class VulkanPhysicalDevice {
    public:
        struct CreationInput {
            // all info required to create a Vulkan physical device
            const std::string name;
            VkInstance instance = nullptr;
            enum class PhysicalDeviceChoice {
                BEST_GPU,
                ANY
            };
            const PhysicalDeviceChoice choice = PhysicalDeviceChoice::BEST_GPU;

            CreationInput(std::string name,
                          VkInstance instance,
                          const PhysicalDeviceChoice &deviceChoice = PhysicalDeviceChoice::BEST_GPU)
                    : name(std::move(name)),
                      instance(instance),
                      choice(deviceChoice) {
            }
        };

        explicit VulkanPhysicalDevice(const CreationInput &creationInput) {
            if (!isValidVulkanPhysicalDeviceCreateInfo(creationInput)) {
                throw std::runtime_error("Unable to create VulkanPhysicalDevice because of invalid creation input!");
            }

            std::vector<VkPhysicalDevice> availablePhysicalDevices = enumeratePhysicalDevices(creationInput.instance);
            if (availablePhysicalDevices.empty()) {
                throw std::runtime_error("No physical devices are available!");
            } else if (availablePhysicalDevices.size() == 1) {
                // todo when logging is implemented: give warning
            }

            VkPhysicalDevice chosenPhysicalDevice = choosePhysicalDevice(availablePhysicalDevices, creationInput);
            if (chosenPhysicalDevice == nullptr) {
                throw std::runtime_error("Unable to choose a physical device!");
            }

            physicalDevice = chosenPhysicalDevice;
        }

        ~VulkanPhysicalDevice() = default;


        VkPhysicalDevice getPhysicalDevice() {
            return physicalDevice;
        }

    private:
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        static bool isValidVulkanPhysicalDeviceCreateInfo(const CreationInput &info);

        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(const VkInstance &instance);

        static VkPhysicalDevice choosePhysicalDevice(const std::vector<VkPhysicalDevice> &availablePhysicalDevices,
                                                     const CreationInput &info);

        static bool isNewBestPhysicalDeviceChoice(VkPhysicalDeviceLimits deviceToCheck,
                                                  VkPhysicalDeviceLimits currentBestDevice);
    };

}// namespace PGraphics
