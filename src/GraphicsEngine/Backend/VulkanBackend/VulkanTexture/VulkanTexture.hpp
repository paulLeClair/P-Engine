//
// Created by paull on 2023-02-02.
//

#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../Scene/SceneResources/Texture/Texture.hpp"

namespace pEngine::girEngine::backend::vulkan {

    /**
     * This should mostly involve just lining up the high-level Scene::Texture info and converting it
     * into the appropriate Vulkan structures;
     *
     * Since in Vulkan we have a few different ways of configuring samplers and what not, I think it makes sense
     * to just focus on the usage of the Texture, and have samplers be paired inherently with them.
     *
     * Plus if we want to allow independent samplers, you can just have that be a special case of the VulkanTexture and
     * Scene::Texture design (ie have the ability to create samplers on their own)
     */
    class VulkanTexture {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;

            VkDevice device;

            VkSamplerCreateInfo samplerCreateInfo;

            VkImageCreateInfo sampledImageCreateInfo;
        };

        explicit VulkanTexture(const CreationInput &creationInput) {
            auto createSamplerResult = vkCreateSampler(device, &creationInput.samplerCreateInfo, nullptr, &sampler);
            if (createSamplerResult != VK_SUCCESS) {
                // TODO - log!
                throw std::runtime_error("Error in VulkanTexture() - unable to create given sampler!");
            }

            auto createSampledImageResult = vkCreateImage(device, &creationInput.sampledImageCreateInfo, nullptr,
                                                          &sampledImage);

            if (createSampledImageResult != VK_SUCCESS) {
                // TODO - log!
                throw std::runtime_error("Error in VulkanTexture() - unable to create given image!");
            }

        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        VkDevice device{};

        VkSampler sampler{};

        VkImage sampledImage{};

    };

} // PGraphics
