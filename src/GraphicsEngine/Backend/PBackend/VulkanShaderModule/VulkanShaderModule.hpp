//
// Created by paull on 2022-06-25.
//

#pragma once

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "../../../../lib/spirv_reflect/spirv_reflect.h"

namespace PGraphics {

/**
 * This should be the PBackend structure for working with Shader Modules in Vulkan, which should also include
 * shader reflection and the like probably;
 *
 * Unlike some of the other Vulkan* classes in the PBackend, this class will probably do a bit more
 * internal work to get you the shader you want (since we have to actually obtain+compile it)
 */
    class VulkanShaderModule {
    public:
        enum class ShaderLanguage {
            GLSL,
            HLSL
        };

        enum class ShaderUsage {
            VERTEX,
            FRAGMENT,
            TESSELLATION_CONTROL,
            TESSELLATION_EVALUATION,
            GEOMETRY
        };

        struct CreateInfo {
            VkDevice device;
            std::string shaderModuleSpirVName;
            ShaderLanguage shaderLanguage = ShaderLanguage::GLSL;
            ShaderUsage shaderUsage;
        };

        static void validateInfo(const CreateInfo &info);

        explicit VulkanShaderModule(const CreateInfo &info);

        ~VulkanShaderModule() {
            vkDestroyShaderModule(parentLogicalDevice, shaderModule, nullptr);
        }

    private:
        std::string shaderModuleSpirVName;
        ShaderLanguage shaderLanguage = ShaderLanguage::GLSL;
        ShaderUsage shaderUsage;
        VkDevice parentLogicalDevice = VK_NULL_HANDLE;
        VkShaderModule shaderModule = VK_NULL_HANDLE;

        /* shader reflection data */
        // I think these VulkanShaderModules will have to be responsible for pretty much all of the shader reflection,
        // which is important because we'll be entirely getting certain input data for the creation of our graphics pipelines
        // from the shader modules, so that the user doesn't have to specify anything other than the shaders they used and
        // resource bindings, and the engine will be able to obtain the rest itself
        struct ShaderReflectionData {
            struct DescriptorSetLayoutInfo {
                unsigned int setNumber = 0;
                VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
                std::vector<VkDescriptorSetLayoutBinding> bindings = {};
            };
            std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos;
            VkVertexInputBindingDescription vertexInputBindingDescription;
            std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
            std::vector<VkPushConstantRange> pushConstantRanges;
        };
        ShaderReflectionData shaderReflectionData;

        static std::string getShaderModuleBinaryPath(const std::string &shaderModuleName);

        static uint32_t getVertexFormatSizeInBytes(VkFormat format);

        static void obtainDescriptorSetLayoutBindingInfos(const SpvReflectShaderModule &reflectShaderModule,
                                                          SpvReflectDescriptorSet *const &reflectedDescriptorSet,
                                                          ShaderReflectionData::DescriptorSetLayoutInfo &info);


        std::vector<uint32_t> getShaderSPIRVCode(const std::string &shaderModuleName);

        void createSPIRVReflectShaderModule(const std::vector<uint32_t> &spirvCode,
                                            SpvReflectShaderModule &reflectShaderModule);

        std::vector<SpvReflectDescriptorSet *>
        enumerateDescriptorSets(SpvReflectShaderModule &reflectShaderModule) const;

        bool reflectShaderInfo(const std::vector<uint32_t> &spirvCode);

        bool reflectedShaderUsageMatchesShaderModule(SpvReflectShaderStageFlagBits bits);

        void reflectDescriptorSetInformation(SpvReflectShaderModule &reflectShaderModule);

        void reflectInputOutputVariables(SpvReflectShaderModule &reflectShaderModule);

        void reflectPushConstantInformation(SpvReflectShaderModule &reflectShaderModule);

        static uint32_t getSpirVModuleSizeInBytes(const std::vector<uint32_t> &shaderSPIRV);
    };

}// namespace PGraphics
