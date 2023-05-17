//
// Created by paull on 2022-06-25.
//

#include "VulkanShaderModule.hpp"

#include <filesystem>
#include <fstream>
#include <set>

namespace PGraphics {

    VulkanShaderModule::VulkanShaderModule(const CreateInfo &info)
            : shaderModuleSpirVName(info.shaderModuleSpirVName),
              shaderUsage(info.shaderUsage),
              shaderLanguage(info.shaderLanguage),
              parentLogicalDevice(info.device) {
        validateInfo(info);

        auto shaderSPIRV = getShaderSPIRVCode(shaderModuleSpirVName);
        VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                               NULL,
                                               0,
                                               getSpirVModuleSizeInBytes(shaderSPIRV),
                                               shaderSPIRV.data()};

        if (vkCreateShaderModule(parentLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shader module " + info.shaderModuleSpirVName);
        }

        bool shaderReflectionSucceeded = reflectShaderInfo(shaderSPIRV);
        if (!shaderReflectionSucceeded) {
            throw std::runtime_error("Unable to complete shader reflection for shader module " + shaderModuleSpirVName);
        }
    }

    uint32_t VulkanShaderModule::getSpirVModuleSizeInBytes(
            const std::vector<uint32_t> &shaderSPIRV) {
        return static_cast<uint32_t>( shaderSPIRV.size() * sizeof(uint32_t));
    }

    std::vector<uint32_t> VulkanShaderModule::getShaderSPIRVCode(const std::string &shaderModuleName) {
        auto shaderModulePath = getShaderModuleBinaryPath(shaderModuleSpirVName);
        std::ifstream file(shaderModulePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open shader file " + shaderModulePath);
        }

        std::vector<uint32_t> shaderSPIRVCode = {};

        auto fileSize = static_cast<uint32_t>( file.tellg());
        unsigned int packedFileSize = fileSize / sizeof(uint32_t);

        shaderSPIRVCode.resize(packedFileSize);
        file.seekg(0);
        file.read((char *) shaderSPIRVCode.data(), fileSize);
        file.close();

        return shaderSPIRVCode;
    }


    std::string VulkanShaderModule::getShaderModuleBinaryPath(const std::string &shaderModuleName) {
        std::filesystem::path currentPath = std::filesystem::current_path().relative_path();
        std::filesystem::path shaderModulePath = std::filesystem::path("C:\\");// TODO - fix this hardcoded value
        for (const auto &currentRelativePathElementItr: currentPath) {
            if (currentRelativePathElementItr.string() == "build") {
                break;
            }

            shaderModulePath += currentRelativePathElementItr;
            shaderModulePath += std::filesystem::path("\\");
        }

        std::string shaderSpirVModuleName = shaderModuleName.substr(0, shaderModuleName.find_first_of('.')) + ".spv";
        shaderModulePath += std::filesystem::path(R"(src\shaders\bin\)" + shaderSpirVModuleName);
        return shaderModulePath.string();
    }

    void VulkanShaderModule::validateInfo(const VulkanShaderModule::CreateInfo &info) {
        if (info.device == NULL) {
            throw std::runtime_error("No device provided in shader module create info!");
        }

        if (info.shaderModuleSpirVName.empty()) {
            throw std::runtime_error("No name provided in shader module create info!");
        }
    }

    bool VulkanShaderModule::reflectShaderInfo(const std::vector<uint32_t> &spirvCode) {
        try {
            SpvReflectShaderModule reflectionModule = {};
            createSPIRVReflectShaderModule(spirvCode, reflectionModule);

            reflectDescriptorSetInformation(reflectionModule);

            reflectInputOutputVariables(reflectionModule);

            reflectPushConstantInformation(reflectionModule);

            // TODO - reflect tessellation info as well
        }
        catch (std::exception &exception) {
            // TODO - log
            return false;
        }

        return true;
    }

    void VulkanShaderModule::reflectPushConstantInformation(SpvReflectShaderModule &reflectShaderModule) {
        uint32_t pushConstantBlockCount = 0;
        SpvReflectResult enumeratePushConstantBlocksResult =
                spvReflectEnumeratePushConstantBlocks(&reflectShaderModule, &pushConstantBlockCount, nullptr);
        if (enumeratePushConstantBlocksResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate push constant blocks!");
        }

        std::vector<SpvReflectBlockVariable *> pushConstantBlocks(pushConstantBlockCount);
        enumeratePushConstantBlocksResult =
                spvReflectEnumeratePushConstantBlocks(&reflectShaderModule, &pushConstantBlockCount,
                                                      pushConstantBlocks.data());
        if (enumeratePushConstantBlocksResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate push constant blocks!");
        }

        shaderReflectionData.pushConstantRanges.clear();
        for (auto pushConstant: pushConstantBlocks) {
            VkPushConstantRange range = {};
            range.size = pushConstant->size;
            range.offset = pushConstant->offset;

            switch (shaderUsage) {
                case (ShaderUsage::VERTEX): {
                    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                }
                    break;
                case (ShaderUsage::FRAGMENT): {
                    range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                }
                    break;
                case (ShaderUsage::TESSELLATION_CONTROL): {
                    range.stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                }
                    break;
                case (ShaderUsage::TESSELLATION_EVALUATION): {
                    range.stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                }
                    break;
                case (ShaderUsage::GEOMETRY): {
                    range.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;
                }
                    break;
            }

            shaderReflectionData.pushConstantRanges.push_back(range);
        }
    }

    void VulkanShaderModule::reflectInputOutputVariables(
            SpvReflectShaderModule &reflectShaderModule) {// enumerate the input variables
        uint32_t inputCount = 0;
        SpvReflectResult enumerateInputVariablesResult =
                spvReflectEnumerateInputVariables(&reflectShaderModule, &inputCount, NULL);
        if (enumerateInputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate input variables for shader " + shaderModuleSpirVName);
        }
        std::vector<SpvReflectInterfaceVariable *> inputs(inputCount);
        enumerateInputVariablesResult = spvReflectEnumerateInputVariables(&reflectShaderModule, &inputCount,
                                                                          inputs.data());
        if (enumerateInputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate input variables for shader " + shaderModuleSpirVName);
        }

        // enumerate output variables
        uint32_t outputCount = 0;
        SpvReflectResult enumerateOutputVariablesResult =
                spvReflectEnumerateOutputVariables(&reflectShaderModule, &outputCount, NULL);
        if (enumerateOutputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate output variables for shader " + shaderModuleSpirVName);
        }
        std::vector<SpvReflectInterfaceVariable *> outputs(outputCount);
        enumerateOutputVariablesResult =
                spvReflectEnumerateOutputVariables(&reflectShaderModule, &outputCount, outputs.data());
        if (enumerateOutputVariablesResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to enumerate output variables for shader " + shaderModuleSpirVName);
        }

        if (shaderUsage == ShaderUsage::VERTEX) {
            shaderReflectionData.vertexInputBindingDescription.binding = 0;
            shaderReflectionData.vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            shaderReflectionData.vertexInputBindingDescription.stride = 0;

            // now we'll build and sort the vertex attribute descriptions array
            shaderReflectionData.vertexInputAttributeDescriptions.resize(inputs.size());
            for (size_t inputVar = 0; inputVar < inputs.size(); inputVar++) {
                const auto &reflectedVar = *inputs[inputVar];
                auto &desc = shaderReflectionData.vertexInputAttributeDescriptions[inputVar];

                desc.location = reflectedVar.location;
                desc.binding = shaderReflectionData.vertexInputBindingDescription
                        .binding;// could use this logic to have the binding be changeable
                desc.format = static_cast<VkFormat>( reflectedVar.format );
                desc.offset = 0;
            }
            std::sort(
                    std::begin(shaderReflectionData.vertexInputAttributeDescriptions),
                    std::end(shaderReflectionData.vertexInputAttributeDescriptions),
                    [&](const VkVertexInputAttributeDescription &first,
                        const VkVertexInputAttributeDescription &second) {
                        // we want to sort in ascending order, so we set this condition
                        return first.location < second.location;
                    });

            for (auto &attribute: shaderReflectionData.vertexInputAttributeDescriptions) {
                attribute.offset = shaderReflectionData.vertexInputBindingDescription.stride;
                shaderReflectionData.vertexInputBindingDescription.stride += getVertexFormatSizeInBytes(
                        attribute.format);
            }
        }
    }

    void VulkanShaderModule::reflectDescriptorSetInformation(SpvReflectShaderModule &reflectShaderModule) {
        auto reflectedDescriptorSets = enumerateDescriptorSets(reflectShaderModule);

        for (const auto &reflectedDescriptorSet: reflectedDescriptorSets) {
            // we reflect descriptor sets one by one:
            const SpvReflectDescriptorSet &reflectionSet = *reflectedDescriptorSet;
            ShaderReflectionData::DescriptorSetLayoutInfo info;

            obtainDescriptorSetLayoutBindingInfos(reflectShaderModule, reflectedDescriptorSet, info);

            info.setNumber = reflectionSet.set;
            info.layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            info.layoutCreateInfo.pNext = nullptr;
            info.layoutCreateInfo.flags = 0;
            info.layoutCreateInfo.bindingCount = static_cast<uint32_t>( info.bindings.size());
            info.layoutCreateInfo.pBindings = info.bindings.data();

            shaderReflectionData.descriptorSetLayoutInfos.push_back(info);
        }
    }

    void VulkanShaderModule::obtainDescriptorSetLayoutBindingInfos(
            const SpvReflectShaderModule &reflectShaderModule,
            SpvReflectDescriptorSet *const &reflectedDescriptorSet,
            VulkanShaderModule::ShaderReflectionData::DescriptorSetLayoutInfo &info) {
        info.bindings.reserve(reflectedDescriptorSet->binding_count);
        for (auto j = 0u; j < info.bindings.size(); j++) {
            const auto &reflectedBinding = *reflectedDescriptorSet->bindings[j];
            auto &infoBinding = info.bindings[j];

            infoBinding.binding = reflectedBinding.binding;
            infoBinding.descriptorType = static_cast<VkDescriptorType>( reflectedBinding.descriptor_type );

            infoBinding.descriptorCount = 1;
            for (uint32_t bindingDim = 0; bindingDim < reflectedBinding.array.dims_count; bindingDim++) {
                infoBinding.descriptorCount *= reflectedBinding.array.dims[bindingDim];
            }

            infoBinding.stageFlags = static_cast<VkShaderStageFlagBits>( reflectShaderModule.shader_stage );
        }
    }

    void VulkanShaderModule::createSPIRVReflectShaderModule(const std::vector<uint32_t> &spirvCode,
                                                            SpvReflectShaderModule &reflectShaderModule) {

        SpvReflectResult createShaderModuleResult =
                spvReflectCreateShaderModule(spirvCode.size() * sizeof(uint32_t), spirvCode.data(),
                                             &reflectShaderModule);

        if (createShaderModuleResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Unable to create SPIRV-Reflect shader module!");
        }

        if (!reflectedShaderUsageMatchesShaderModule(reflectShaderModule.shader_stage)) {
            throw std::runtime_error(
                    "Reflected shader module usage does not match user-specified shader module usage!");
        }
    }

    std::vector<SpvReflectDescriptorSet *> VulkanShaderModule::enumerateDescriptorSets(
            SpvReflectShaderModule &reflectShaderModule) const {
        uint32_t count = 0;
        SpvReflectResult enumerateDescriptorSetsResult =
                spvReflectEnumerateDescriptorSets(&reflectShaderModule, &count, NULL);
        if (enumerateDescriptorSetsResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error(
                    "Unable to enumerate shader " + shaderModuleSpirVName + "'s descriptor sets during reflection!");
        }

        std::vector<SpvReflectDescriptorSet *> sets(count);
        enumerateDescriptorSetsResult = spvReflectEnumerateDescriptorSets(&reflectShaderModule, &count, sets.data());
        if (enumerateDescriptorSetsResult != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error(
                    "Unable to enumerate shader " + shaderModuleSpirVName + "'s descriptor sets during reflection!");
        }

        return sets;
    }

    bool VulkanShaderModule::reflectedShaderUsageMatchesShaderModule(SpvReflectShaderStageFlagBits bits) {
        ShaderUsage reflectedShaderUsage;
        switch (bits) {
            case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                reflectedShaderUsage = ShaderUsage::VERTEX;
                break;
            case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                reflectedShaderUsage = ShaderUsage::FRAGMENT;
                break;
            case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                reflectedShaderUsage = ShaderUsage::TESSELLATION_CONTROL;
                break;
            case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                reflectedShaderUsage = ShaderUsage::TESSELLATION_EVALUATION;
                break;
            case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                reflectedShaderUsage = ShaderUsage::GEOMETRY;
                break;
            default:
                throw std::runtime_error("Unable to determine reflected shader stage!");
        }

        return reflectedShaderUsage == shaderUsage;
    }

/**
 * Returns the size (in bytes) of the given Vulkan data format.
 *
 * Credits for this function: SPIRV-Reflect repo
 * @param format
 * @return
 */
    uint32_t VulkanShaderModule::getVertexFormatSizeInBytes(VkFormat format) {
        uint32_t result = 0;
        switch (format) {
            case VK_FORMAT_UNDEFINED:
                result = 0;
                break;
            case VK_FORMAT_R4G4_UNORM_PACK8:
                result = 1;
                break;
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
                result = 2;
                break;
            case VK_FORMAT_R8_UNORM:
                result = 1;
                break;
            case VK_FORMAT_R8_SNORM:
                result = 1;
                break;
            case VK_FORMAT_R8_USCALED:
                result = 1;
                break;
            case VK_FORMAT_R8_SSCALED:
                result = 1;
                break;
            case VK_FORMAT_R8_UINT:
                result = 1;
                break;
            case VK_FORMAT_R8_SINT:
                result = 1;
                break;
            case VK_FORMAT_R8_SRGB:
                result = 1;
                break;
            case VK_FORMAT_R8G8_UNORM:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SNORM:
                result = 2;
                break;
            case VK_FORMAT_R8G8_USCALED:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SSCALED:
                result = 2;
                break;
            case VK_FORMAT_R8G8_UINT:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SINT:
                result = 2;
                break;
            case VK_FORMAT_R8G8_SRGB:
                result = 2;
                break;
            case VK_FORMAT_R8G8B8_UNORM:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SNORM:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_USCALED:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SSCALED:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_UINT:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SINT:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8_SRGB:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_UNORM:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SNORM:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_USCALED:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SSCALED:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_UINT:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SINT:
                result = 3;
                break;
            case VK_FORMAT_B8G8R8_SRGB:
                result = 3;
                break;
            case VK_FORMAT_R8G8B8A8_UNORM:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SNORM:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_USCALED:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_UINT:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SINT:
                result = 4;
                break;
            case VK_FORMAT_R8G8B8A8_SRGB:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_UNORM:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SNORM:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_USCALED:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_UINT:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SINT:
                result = 4;
                break;
            case VK_FORMAT_B8G8R8A8_SRGB:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_R16_UNORM:
                result = 2;
                break;
            case VK_FORMAT_R16_SNORM:
                result = 2;
                break;
            case VK_FORMAT_R16_USCALED:
                result = 2;
                break;
            case VK_FORMAT_R16_SSCALED:
                result = 2;
                break;
            case VK_FORMAT_R16_UINT:
                result = 2;
                break;
            case VK_FORMAT_R16_SINT:
                result = 2;
                break;
            case VK_FORMAT_R16_SFLOAT:
                result = 2;
                break;
            case VK_FORMAT_R16G16_UNORM:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SNORM:
                result = 4;
                break;
            case VK_FORMAT_R16G16_USCALED:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SSCALED:
                result = 4;
                break;
            case VK_FORMAT_R16G16_UINT:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SINT:
                result = 4;
                break;
            case VK_FORMAT_R16G16_SFLOAT:
                result = 4;
                break;
            case VK_FORMAT_R16G16B16_UNORM:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SNORM:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_USCALED:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SSCALED:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_UINT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SINT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16_SFLOAT:
                result = 6;
                break;
            case VK_FORMAT_R16G16B16A16_UNORM:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SNORM:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_USCALED:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SSCALED:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_UINT:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SINT:
                result = 8;
                break;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R32_UINT:
                result = 4;
                break;
            case VK_FORMAT_R32_SINT:
                result = 4;
                break;
            case VK_FORMAT_R32_SFLOAT:
                result = 4;
                break;
            case VK_FORMAT_R32G32_UINT:
                result = 8;
                break;
            case VK_FORMAT_R32G32_SINT:
                result = 8;
                break;
            case VK_FORMAT_R32G32_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R32G32B32_UINT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32_SINT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32_SFLOAT:
                result = 12;
                break;
            case VK_FORMAT_R32G32B32A32_UINT:
                result = 16;
                break;
            case VK_FORMAT_R32G32B32A32_SINT:
                result = 16;
                break;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                result = 16;
                break;
            case VK_FORMAT_R64_UINT:
                result = 8;
                break;
            case VK_FORMAT_R64_SINT:
                result = 8;
                break;
            case VK_FORMAT_R64_SFLOAT:
                result = 8;
                break;
            case VK_FORMAT_R64G64_UINT:
                result = 16;
                break;
            case VK_FORMAT_R64G64_SINT:
                result = 16;
                break;
            case VK_FORMAT_R64G64_SFLOAT:
                result = 16;
                break;
            case VK_FORMAT_R64G64B64_UINT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64_SINT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64_SFLOAT:
                result = 24;
                break;
            case VK_FORMAT_R64G64B64A64_UINT:
                result = 32;
                break;
            case VK_FORMAT_R64G64B64A64_SINT:
                result = 32;
                break;
            case VK_FORMAT_R64G64B64A64_SFLOAT:
                result = 32;
                break;
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
                result = 4;
                break;
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
                result = 4;
                break;

            default:
                break;
        }
        return result;
    }

}// namespace PGraphics