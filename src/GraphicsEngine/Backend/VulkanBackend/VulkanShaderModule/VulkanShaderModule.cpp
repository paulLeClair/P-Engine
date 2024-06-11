//
// Created by paull on 2022-06-25.
//

#include "VulkanShaderModule.hpp"

#include <filesystem>
#include <fstream>
#include <algorithm>

namespace pEngine::girEngine::backend::vulkan {
    VulkanShaderModule::VulkanShaderModule(const CreationInput &info)
            : shaderModuleSpirVName(info.shaderModuleSpirVName),
              uniqueIdentifier(info.uniqueIdentifier),
              shaderUsage(info.shaderUsage),
              parentLogicalDevice(info.device),
              shaderEntryPointName(info.shaderEntryPointName) {
        validateInfo(info);

        spirVByteCode = readSpirVByteCode(shaderModuleSpirVName);
        VkShaderModuleCreateInfo createInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                nullptr,
                0,
                getSpirVModuleSizeInBytes(spirVByteCode),
                spirVByteCode.data()
        };

        if (vkCreateShaderModule(parentLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shader module " + info.shaderModuleSpirVName);
        }
    }

    uint32_t VulkanShaderModule::getSpirVModuleSizeInBytes(
            const std::vector<uint32_t> &shaderSPIRV) {
        return static_cast<uint32_t>(shaderSPIRV.size() * sizeof(uint32_t));
    }

    std::vector<uint32_t> VulkanShaderModule::readSpirVByteCode(const std::string &shaderModuleName) {
        auto shaderModulePath = getShaderModuleBinaryPath(shaderModuleSpirVName);
        std::ifstream file(shaderModulePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open shader file " + shaderModulePath);
        }

        std::vector<uint32_t> shaderSPIRVCode = {};

        auto fileSize = static_cast<uint32_t>(file.tellg());
        unsigned int packedFileSize = fileSize / sizeof(uint32_t);

        shaderSPIRVCode.resize(packedFileSize);
        file.seekg(0);
        file.read((char *) shaderSPIRVCode.data(), fileSize);
        file.close();

        return shaderSPIRVCode;
    }


    /**
     * This basically builds a path string corresponding to a compiled SPIR-V shader binary
     * file called `{shaderModuleName}.spv`  inside the `{repo}/src/shaders/bin/` directory
     * @param shaderModuleName
     * @return
     */
    std::string VulkanShaderModule::getShaderModuleBinaryPath(const std::string &shaderModuleName) {
        const std::filesystem::path currentPath = std::filesystem::current_path().relative_path();
        auto shaderModulePath = std::filesystem::path("C:\\"); // TODO - fix this hardcoded value
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

    void VulkanShaderModule::validateInfo(const VulkanShaderModule::CreationInput &info) {
        if (info.device == nullptr) {
            throw std::runtime_error("No device provided in shader module create info!");
        }

        if (info.shaderModuleSpirVName.empty()) {
            throw std::runtime_error("No name provided in shader module create info!");
        }
    }

    const std::vector<uint32_t> &VulkanShaderModule::getSpirVByteCode() const {
        return spirVByteCode;
    }

    const gir::ShaderModuleIR::ShaderUsage &VulkanShaderModule::getUsage() const {
        return shaderUsage;
    }

    VkShaderModule VulkanShaderModule::getVkShaderModule() const {
        return shaderModule;
    }

    const std::string &VulkanShaderModule::getShaderEntryPointName() const {
        return shaderEntryPointName;
    }
} // namespace PGraphics
