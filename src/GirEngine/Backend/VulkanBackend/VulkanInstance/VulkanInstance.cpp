//
// Created by paull on 2022-06-19.
//

#include "VulkanInstance.hpp"

#include <stdexcept>

namespace pEngine::girEngine::backend::vulkan {
    const std::map<VulkanInstance::SupportedInstanceExtension, std::string>
    VulkanInstance::SUPPORTED_INSTANCE_EXTENSION_NAMES = {
        {SupportedInstanceExtension::SURFACE_EXTENSION, VK_KHR_SURFACE_EXTENSION_NAME},
        {
            SupportedInstanceExtension::WINDOWS_SURFACE_EXTENSION,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        },
        {SupportedInstanceExtension::XLIB_SURFACE_EXTENSION, VK_KHR_XLIB_SURFACE_EXTENSION_NAME},

        {SupportedInstanceExtension::DEBUG_UTILS, VK_EXT_DEBUG_UTILS_EXTENSION_NAME},
        // TODO - any other instance extensions we want to support
    };


    const std::map<VulkanInstance::SupportedLayers, std::string> VulkanInstance::SUPPORTED_INSTANCE_LAYER_NAMES = {
        {SupportedLayers::VALIDATION_LAYER, "VK_LAYER_KHRONOS_validation"},
        // TODO - any other layers we want to support
    };

    /**
     *
     * @param instanceCreationInput
     */
    VulkanInstance::VulkanInstance(const CreationInput &instanceCreationInput)
        : instance(VK_NULL_HANDLE),
          vulkanEngineVersion(instanceCreationInput.vulkanEngineVersion),
          vulkanAppVersion(instanceCreationInput.vulkanAppVersion),
          vulkanApiVersion(instanceCreationInput.vulkanApiVersion) {
        // initialize fields for instance extensions and layers
        initializeEnabledExtensions(instanceCreationInput);
        initializeEnabledLayers(instanceCreationInput);

        // create the vulkan instance
        if (createSingleInstance(instanceCreationInput) != CreateInstanceResult::SUCCESS) {
            throw std::runtime_error("Failed to create singular instance!");
        }
    }

    void VulkanInstance::initializeEnabledLayers(
        const CreationInput &instanceCreationInput) {
        // convert layers from enum class value to string
        std::vector<const char *> enabledLayersInStringForm = {};
        for (const auto &enabledLayer: instanceCreationInput.enabledInstanceLayers) {
            enabledLayersInStringForm.push_back(SUPPORTED_INSTANCE_LAYER_NAMES.at(enabledLayer).c_str());
        }
        enabledInstanceLayers = enabledLayersInStringForm;
    }

    void VulkanInstance::initializeEnabledExtensions(
        const CreationInput &instanceCreationInput) {
        // convert extensions from enum class value to string
        std::vector<const char *> enabledExtensionsInStringForm = {};
        for (const auto &enabledExtension: instanceCreationInput.enabledInstanceExtensions) {
            enabledExtensionsInStringForm.push_back(SUPPORTED_INSTANCE_EXTENSION_NAMES.at(enabledExtension).c_str());
        }
        enabledInstanceExtensions = enabledExtensionsInStringForm;
    }

    VulkanInstance::~VulkanInstance() {
        vkDestroyInstance(instance, nullptr);
    }


    VulkanInstance::CreateInstanceResult VulkanInstance::createSingleInstance(
        const CreationInput &info) {
        VkApplicationInfo applicationInfo = getVkApplicationInfo(info.vulkanAppName,
                                                                 info.vulkanEngineName,
                                                                 info.vulkanAppVersion,
                                                                 info.vulkanEngineVersion,
                                                                 info.vulkanApiVersion);

        VkInstanceCreateInfo singleInstanceCreateInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            static_cast<uint32_t>(enabledInstanceLayers.size()),
            enabledInstanceLayers.data(),
            static_cast<uint32_t>(enabledInstanceExtensions.size()),
            enabledInstanceExtensions.data()
        };

        std::vector validationFeatureDisableFlags = {
            VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT
        };
        VkValidationFeaturesEXT validationFeatures = {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .pNext = nullptr,
            .disabledValidationFeatureCount = static_cast<uint32_t>(validationFeatureDisableFlags.size()),
            .pDisabledValidationFeatures = validationFeatureDisableFlags.data(),
        };


        singleInstanceCreateInfo.pNext = &validationFeatures;

        if (vkCreateInstance(&singleInstanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
            return CreateInstanceResult::FAILURE;
        }

        return CreateInstanceResult::SUCCESS;
    }

    VkApplicationInfo
    VulkanInstance::getVkApplicationInfo(const std::string &appName,
                                         const std::string &engineName,
                                         unsigned int appVersion,
                                         unsigned int engineVersion,
                                         unsigned int vulkanApiVersion) {
        return {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            appName.c_str(),
            appVersion, // TODO - make this version configurable
            engineName.c_str(),
            engineVersion, // TODO - make this version configurable
            vulkanApiVersion // TODO - make this configurable too
        };
    }
} // namespace PGraphics
