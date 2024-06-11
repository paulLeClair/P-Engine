//
// Created by paull on 2022-06-19.
//
#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <set>

namespace pEngine::girEngine::backend::appContext::vulkan {
    class VulkanInstance {
    public:
        /**
         * This is one of the specifically instance-level extensions out of the list
         * here:
         * https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap56.html
         *
         * This includes stuff that is required as part of raytracing so that's probably
         * going to come in handy later! We can add one for each extension I think.
         */
        enum class SupportedInstanceExtension {
            UNKNOWN,
            SURFACE_EXTENSION,
            WINDOWS_SURFACE_EXTENSION,
        };

        /**
         *
         */
        enum class SupportedLayers {
            UNKNOWN,
            VALIDATION_LAYER
        };

        /**
         * TODO - add vulkan instance configuration here
         */
        struct CreationInput {
            /**
             * This is the set of instance extensions you want to enable for a Vulkan backend.
             */
            std::vector<SupportedInstanceExtension> enabledInstanceExtensions = {};

            /**
             * This is the set of layers you want to enable for a Vulkan backend.
             */
            std::vector<SupportedLayers> enabledInstanceLayers;

            /**
             * This is the engine name that gets passed into the Vulkan instance
             */
            const std::string &vulkanEngineName;

            /**
             * This is the application name that gets passed into the Vulkan instance
             */
            const std::string &vulkanAppName;

            /**
             * This gets fed into the app version parameter of the Vulkan Instance
             */
            unsigned int vulkanAppVersion = 0;

            /**
              * This gets fed into the app version parameter of the Vulkan Instance
              */
            unsigned int vulkanEngineVersion = 0;

            /**
             * This is the version of the overall vulkan API being used; This engine supports Vulkan 1.3+
             *
             * Note: you can make a version uint using the vulkan version macro: VK_API_VERSION_1_3 or VK_MAKE_API_VERSION(0, 1, 3, 0)
             */
            unsigned int vulkanApiVersion = 0;
        };

        explicit VulkanInstance(const CreationInput &creationInput);

        ~VulkanInstance();

        [[nodiscard]] const VkInstance &getVkInstance() const {
            return instance;
        }

        [[nodiscard]] const std::vector<const char *> &getEnabledInstanceExtensions() const {
            return enabledInstanceExtensions;
        }

        [[nodiscard]] const std::vector<const char *> &getEnabledInstanceLayers() const {
            return enabledInstanceLayers;
        }

        [[nodiscard]] unsigned int getVulkanAppVersion() const {
            return vulkanAppVersion;
        }

        [[nodiscard]] unsigned int getVulkanEngineVersion() const {
            return vulkanEngineVersion;
        }

        [[nodiscard]] unsigned int getVulkanApiVersion() const {
            return vulkanApiVersion;
        }

    private:
        /**
         * This defines the mapping from supported instance extensions to the correct string required by Vulkan
         */
        const static std::map<SupportedInstanceExtension, std::string> SUPPORTED_INSTANCE_EXTENSION_NAMES;

        /**
         * This defines the mapping from supported instance extensions to the correct string required by Vulkan
         */
        const static std::map<SupportedLayers, std::string> SUPPORTED_INSTANCE_LAYER_NAMES;

        VkInstance instance;

        enum class CreateInstanceResult {
            SUCCESS,
            FAILURE
        };

        CreateInstanceResult createSingleInstance(const CreationInput &info);

        /* INSTANCE CREATION */

        std::vector<const char *> enabledInstanceExtensions;

        std::vector<const char *> enabledInstanceLayers;

        unsigned int vulkanAppVersion, vulkanEngineVersion, vulkanApiVersion;

        void initializeEnabledExtensions(const CreationInput &instanceCreationInput);

        void initializeEnabledLayers(const CreationInput &instanceCreationInput);

        static VkApplicationInfo
        getVkApplicationInfo(const std::string &appName,
                             const std::string &engineName,
                             unsigned int appVersion,
                             unsigned int engineVersion,
                             unsigned int vulkanApiVersion);
    };
} // namespace PGraphics
