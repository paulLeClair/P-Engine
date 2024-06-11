//
// Created by paull on 2023-02-02.
//

#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::backend::vulkan {

    class VulkanPushConstant {
    public:
        struct CreationInput {
            const std::string &name;
            const util::UniqueIdentifier &uniqueIdentifier;

            VkShaderStageFlags shaderStageFlags;
            uint32_t offset;
            uint32_t size;
        };

        explicit VulkanPushConstant(const CreationInput &creationInput) : name(creationInput.name),
                                                                          uniqueIdentifier(
                                                                                  creationInput.uniqueIdentifier),
                                                                          pushConstantRange(
                                                                                  {creationInput.shaderStageFlags,
                                                                                   creationInput.offset,
                                                                                   creationInput.size}) {
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUniqueIdentifier() const {
            return uniqueIdentifier;
        }

        [[nodiscard]] const VkPushConstantRange &getPushConstantRange() const {
            return pushConstantRange;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        VkPushConstantRange pushConstantRange;
    };

} // PGraphics
