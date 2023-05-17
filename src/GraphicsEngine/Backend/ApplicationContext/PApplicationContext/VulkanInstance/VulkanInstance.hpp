//
// Created by paull on 2022-06-19.
//

#pragma once

// seems kinda bad that I have to remember to include this #define before every time I include vulkan...
// maybe I'll look into it but a Google search hasn't revealed many options
#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace PGraphics {

    class VulkanInstance {
    public:
        struct CreationInput {
            std::string name;
        };

        explicit VulkanInstance(const CreationInput &creationInput);

        ~VulkanInstance();

        [[nodiscard]] VkInstance getVkInstance() const {
            return instance;
        }

    private:
        VkInstance instance;

        enum class CreateInstanceResult {
            SUCCESS,
            FAILURE
        };

        CreateInstanceResult createSingleInstance(const CreationInput &info);

        /* INSTANCE CREATION */
        static VkApplicationInfo generateApplicationInfo();

        static std::vector<const char *> generateInstanceLayerConfiguration();

        static std::vector<const char *> generateInstanceExtensionConfiguration();
    };

}// namespace PGraphics
