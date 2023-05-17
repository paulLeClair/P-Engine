//
// Created by paull on 2022-06-18.
//

#pragma once

/**
 * BIG TODO (probably in a future issue) -> add a lot of support for mesh shading techniques! I think they can be used for both
 * traditional pipelines and raytracing, at least on modern hardware (GPUs that are < 3-4 years old should support it hopefully)
 *
 *
 */
class ShaderModule {
public:
    ~ShaderModule() = default;

    enum class EnabledShaderUsageBits {
        VERTEX_SHADER = 1,
        FRAGMENT_SHADER = 2,
        TESSELLATION_EVALUATION_SHADER = 4,
        TESSELLATION_CONTROL_SHADER = 8,
        GEOMETRY_SHADER = 16,
        MESH_TASK_SHADER = 32,
        MESH_SHADER = 64
    };
    using EnabledShaderUsages = unsigned int;

    enum class ShaderFileType {
        HLSL,
        GLSL,
        MSL
    };

    struct CreateInfo {
        std::string name;// name of overall shader module
        std::string shaderFileName;
        EnabledShaderUsages enabledShaderUsages;
        ShaderFileType shaderFileType;
    };

    explicit ShaderModule(const CreateInfo &createInfo) : name(createInfo.name),
                                                          shaderFileName(createInfo.shaderFileName),
                                                          enabledShaderUsages(createInfo.enabledShaderUsages),
                                                          shaderFileType(createInfo.shaderFileType) {

    }

    [[nodiscard]] const std::string &getName() const {
        return name;
    }

    [[nodiscard]] const std::string &getShaderFileName() const {
        return shaderFileName;
    }

    [[nodiscard]] EnabledShaderUsages getEnabledShaderUsages() const {
        return enabledShaderUsages;
    }

    [[nodiscard]] ShaderFileType getShaderFileType() const {
        return shaderFileType;
    }


protected:
    const std::string name;
    const std::string shaderFileName;
    const EnabledShaderUsages enabledShaderUsages;
    const ShaderFileType shaderFileType;
};
