//
// Created by paull on 2022-06-25.
//

#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "../../../GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp"

#include "../../../../lib/spirv_reflect/spirv_reflect.h"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * This should be the PBackend structure for working with Shader Modules in Vulkan, which should also include
     * shader reflection and the like probably;
     *
     * Unlike some of the other Vulkan* classes in the PBackend, this class will probably do a bit more
     * internal work to get you the shader you want (since we have to actually obtain+compile it)
     */
    class VulkanShaderModule {
    public:
        struct CreationInput {
            VkDevice device;
            std::string shaderModuleSpirVName;
            util::UniqueIdentifier uniqueIdentifier;
            gir::ShaderModuleIR::ShaderUsage shaderUsage;
            std::string shaderEntryPointName;
        };

        static void validateInfo(const CreationInput &info);

        explicit VulkanShaderModule(const CreationInput &info);

        ~VulkanShaderModule() {
            vkDestroyShaderModule(parentLogicalDevice, shaderModule, nullptr);
        }

        static uint32_t getSpirVModuleSizeInBytes(const std::vector<uint32_t> &shaderSPIRV);

        [[nodiscard]] const std::vector<uint32_t> &getSpirVByteCode() const;

        [[nodiscard]] const gir::ShaderModuleIR::ShaderUsage &getUsage() const;

        [[nodiscard]] const std::string &getShaderModuleSpirVName() const {
            return shaderModuleSpirVName;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uniqueIdentifier;
        }

        [[nodiscard]] VkShaderModule getVkShaderModule() const;

        [[nodiscard]] const std::string &getShaderEntryPointName() const;

        static VkSpecializationInfo getVkSpecializationInfo() {
            // TODO - figure out what this is even for ... we can probably get away with leaving this for now tho
            return {};
        }

        [[nodiscard]] static const VkSpecializationInfo &getSpecializationInfo() {
            return specializationInfo;
        }

    private:
        constexpr static const VkSpecializationInfo specializationInfo{
                0,
                nullptr,
                0,
                nullptr
        };

        std::string shaderModuleSpirVName;
        util::UniqueIdentifier uniqueIdentifier;
        gir::ShaderModuleIR::ShaderUsage shaderUsage;
        VkDevice parentLogicalDevice = VK_NULL_HANDLE;
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        std::vector<uint32_t> spirVByteCode = {};

        std::string shaderEntryPointName;

        static std::string getShaderModuleBinaryPath(const std::string &shaderModuleName);

        std::vector<uint32_t> readSpirVByteCode(const std::string &shaderModuleName);
    };
} // namespace PGraphics
