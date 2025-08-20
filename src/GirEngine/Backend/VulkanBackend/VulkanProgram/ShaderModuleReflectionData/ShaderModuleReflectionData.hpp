//
// Created by paull on 2023-11-22.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace pEngine::girEngine::backend::vulkan::program {

    /**
     * As the name implies (hopefully), this should contain the data reflected from a SPIR-V shader module.
     */
    /* shader reflection data */
    struct ShaderModuleReflectionData {
        std::string shaderModuleName;

        gir::ShaderModuleIR::ShaderUsage shaderModuleUsage = gir::ShaderModuleIR::ShaderUsage::UNKNOWN;

        struct DescriptorSetLayoutInfo {
            unsigned setNumber = 0;
            VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
            std::vector<VkDescriptorSetLayoutBinding> bindings = {};
        };
        std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos;

        [[maybe_unused]] std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {};

        std::vector<VkPushConstantRange> pushConstantRanges;
    };

} // program
