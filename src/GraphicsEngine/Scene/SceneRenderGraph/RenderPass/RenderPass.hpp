#pragma once

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class RenderPass {
public:
    ~RenderPass() = default;

    struct ResourceBindings {
        /* IMAGES */
        std::vector<std::string> colorOutputs;

        std::vector<std::string> depthStencilInputs;
        std::vector<std::string> depthStencilOutputs;

        std::vector<std::string> storageImageInputs;
        std::vector<std::string> storageImageOutputs;

        std::vector<std::string> textureImageInputs;

        /* BUFFERS */
        std::vector<std::string> uniformBufferInputs;

        std::vector<std::string> uniformTexelBufferInputs;

        std::vector<std::string> storageBufferInputs;
        std::vector<std::string> storageBufferOutputs;

        std::vector<std::string> storageTexelBufferInputs;
        std::vector<std::string> storageTexelBufferOutputs;
    };

    struct ShaderBindings {
        std::string vertexShader;
        std::string fragmentShader;
        std::string tessellationEvaluationShader;
        std::string tessellationControlShader;
        std::string geometryShader;
    };

    enum class AddBindingResult {
        SUCCESS,
        FAILURE
    };

    /* SceneResource Bindings Interface */
    virtual AddBindingResult addColorOutputBinding(const std::string &colorOutputImageName) = 0;

    virtual AddBindingResult addDepthStencilInputBinding(const std::string &colorDepthStencilInputImageName) = 0;

    virtual AddBindingResult addDepthStencilOutputBinding(const std::string &colorDepthStencilOutputImageName) = 0;

    virtual AddBindingResult addStorageImageInputBinding(const std::string &storageImageInputImageName) = 0;

    virtual AddBindingResult addStorageImageOutputBinding(const std::string &storageImageOutputImageName) = 0;

    virtual AddBindingResult addUniformBufferInputBinding(const std::string &uniformBufferInputImageName) = 0;

    virtual AddBindingResult addUniformTexelBufferInputBinding(const std::string &uniformTexelBufferInputImageName) = 0;

    virtual AddBindingResult addStorageBufferInputBinding(const std::string &storageBufferInputBufferName) = 0;

    virtual AddBindingResult addStorageBufferOutputBinding(const std::string &storageBufferOutputBufferName) = 0;

    virtual AddBindingResult
    addStorageTexelBufferInputBinding(const std::string &storageTexelBufferInputBufferName) = 0;

    virtual AddBindingResult
    addStorageTexelBufferOutputBinding(const std::string &storageTexelBufferOutputBufferName) = 0;

    /* Shader Bindings Interface */
    virtual AddBindingResult setVertexShaderBinding(const std::string &vertexShaderModuleName) = 0;

    virtual AddBindingResult setFragmentShaderBinding(const std::string &fragmentShaderModuleName) = 0;

    virtual AddBindingResult
    setTessellationEvaluationShaderBinding(const std::string &tessellationEvaluationShaderModuleName) = 0;

    virtual AddBindingResult
    setTessellationControlShaderBinding(const std::string &tessellationControlShaderModuleName) = 0;

    virtual AddBindingResult setGeometryShaderBinding(const std::string &geometryShaderModuleName) = 0;

    [[nodiscard]] virtual const std::string &getName() const = 0;

    [[nodiscard]] virtual const ResourceBindings &getResourceBindings() const = 0;

    [[nodiscard]] virtual const ShaderBindings &getShaderBindings() const = 0;

};
