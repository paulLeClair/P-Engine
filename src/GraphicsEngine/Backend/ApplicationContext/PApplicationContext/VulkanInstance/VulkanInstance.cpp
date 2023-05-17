//
// Created by paull on 2022-06-19.
//

#include "VulkanInstance.hpp"

#include <stdexcept>

namespace PGraphics {

/**
 *
 * @param instanceCreationInput
 */
    VulkanInstance::VulkanInstance(const CreationInput &instanceCreationInput) {
        if (createSingleInstance(VulkanInstance::CreationInput()) !=
            VulkanInstance::CreateInstanceResult::SUCCESS) {
            throw std::runtime_error("Failed to create singular instance!");
        }
    }

    VulkanInstance::~VulkanInstance() {
        vkDestroyInstance(instance, nullptr);
    }


    VulkanInstance::CreateInstanceResult VulkanInstance::createSingleInstance(
            const VulkanInstance::CreationInput &info) {
        /* create singular instance */
        VkApplicationInfo applicationInfo = generateApplicationInfo();
        std::vector<const char *> enabledInstanceLayers = generateInstanceLayerConfiguration();
        std::vector<const char *> enabledInstanceExtensions = generateInstanceExtensionConfiguration();
        VkInstanceCreateInfo singleInstanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                         NULL,// NOLINT(modernize-use-nullptr)
                                                         0,
                                                         &applicationInfo,
                                                         static_cast<uint32_t>( enabledInstanceLayers.size()),
                                                         enabledInstanceLayers.data(),
                                                         static_cast<uint32_t>( enabledInstanceExtensions.size()),
                                                         enabledInstanceExtensions.data()};

        if (vkCreateInstance(&singleInstanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
            return VulkanInstance::CreateInstanceResult::FAILURE;
        }

        return VulkanInstance::CreateInstanceResult::SUCCESS;
    }

    VkApplicationInfo VulkanInstance::generateApplicationInfo() {
        // TODO - add inputs to this function to configure things
        auto appName = "PGame";
        auto engineName = "PEngineCore";
        return {VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, appName, 0, engineName, 0, VK_VERSION_1_3};
    }

    std::vector<const char *> VulkanInstance::generateInstanceLayerConfiguration() {
        // TODO - make this dynamic (future issue?)
        return {"VK_LAYER_KHRONOS_validation"};
    }

    std::vector<const char *> VulkanInstance::generateInstanceExtensionConfiguration() {
        // TODO - make this dynamic (future issue?)
        return {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    }


}// namespace PGraphics