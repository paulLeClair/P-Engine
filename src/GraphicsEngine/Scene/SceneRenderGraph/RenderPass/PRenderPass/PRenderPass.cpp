#include "PRenderPass.hpp"

#include <algorithm>

namespace PGraphics {
    RenderPass::AddBindingResult PRenderPass::addColorOutputBinding(const std::string &colorOutputImageName) {
        if (!isValidResourceBindingInput(colorOutputImageName, resourceBindings->colorOutputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }


        resourceBindings->colorOutputs.push_back(colorOutputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param colorDepthStencilInputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addDepthStencilInputBinding(
            const std::string &colorDepthStencilInputImageName) {
        if (!isValidResourceBindingInput(colorDepthStencilInputImageName, resourceBindings->depthStencilInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->depthStencilInputs.push_back(colorDepthStencilInputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param colorDepthStencilOutputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addDepthStencilOutputBinding(
            const std::string &colorDepthStencilOutputImageName) {
        if (!isValidResourceBindingInput(colorDepthStencilOutputImageName, resourceBindings->depthStencilOutputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->depthStencilOutputs.push_back(colorDepthStencilOutputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageImageInputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageImageInputBinding(
            const std::string &storageImageInputImageName) {
        if (!isValidResourceBindingInput(storageImageInputImageName, resourceBindings->storageImageInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageImageInputs.push_back(storageImageInputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageImageOutputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageImageOutputBinding(
            const std::string &storageImageOutputImageName) {
        if (!isValidResourceBindingInput(storageImageOutputImageName, resourceBindings->storageImageOutputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageImageOutputs.push_back(storageImageOutputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param uniformBufferInputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addUniformBufferInputBinding(
            const std::string &uniformBufferInputImageName) {
        if (!isValidResourceBindingInput(uniformBufferInputImageName, resourceBindings->uniformBufferInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->uniformBufferInputs.push_back(uniformBufferInputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param uniformTexelBufferInputImageName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addUniformTexelBufferInputBinding(
            const std::string &uniformTexelBufferInputImageName) {
        if (!isValidResourceBindingInput(uniformTexelBufferInputImageName,
                                         resourceBindings->uniformTexelBufferInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->uniformTexelBufferInputs.push_back(uniformTexelBufferInputImageName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageBufferInputBufferName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageBufferInputBinding(
            const std::string &storageBufferInputBufferName) {
        if (!isValidResourceBindingInput(storageBufferInputBufferName, resourceBindings->storageBufferInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageBufferInputs.push_back(storageBufferInputBufferName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageBufferOutputBufferName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageBufferOutputBinding(
            const std::string &storageBufferOutputBufferName) {
        if (!isValidResourceBindingInput(storageBufferOutputBufferName, resourceBindings->storageBufferOutputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageBufferOutputs.push_back(storageBufferOutputBufferName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageTexelBufferInputBufferName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageTexelBufferInputBinding(
            const std::string &storageTexelBufferInputBufferName) {
        if (!isValidResourceBindingInput(storageTexelBufferInputBufferName,
                                         resourceBindings->storageTexelBufferInputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageTexelBufferInputs.push_back(storageTexelBufferInputBufferName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param storageTexelBufferOutputBufferName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::addStorageTexelBufferOutputBinding(
            const std::string &storageTexelBufferOutputBufferName) {
        if (!isValidResourceBindingInput(storageTexelBufferOutputBufferName,
                                         resourceBindings->storageTexelBufferOutputs)) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        resourceBindings->storageTexelBufferOutputs.push_back(storageTexelBufferOutputBufferName);
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param vertexShaderModuleName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::setVertexShaderBinding(const std::string &vertexShaderModuleName) {
        if (!isValidResourceBindingInput(vertexShaderModuleName, {shaderBindings->vertexShader})) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        shaderBindings->vertexShader = vertexShaderModuleName;
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param fragmentShaderModuleName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::setFragmentShaderBinding(const std::string &fragmentShaderModuleName) {
        if (!isValidResourceBindingInput(fragmentShaderModuleName, {shaderBindings->fragmentShader})) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        shaderBindings->fragmentShader = fragmentShaderModuleName;
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param tessellationEvaluationShaderModuleName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::setTessellationEvaluationShaderBinding(
            const std::string &tessellationEvaluationShaderModuleName) {
        if (!isValidResourceBindingInput(tessellationEvaluationShaderModuleName,
                                         {shaderBindings->tessellationEvaluationShader})) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        shaderBindings->tessellationEvaluationShader = tessellationEvaluationShaderModuleName;
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param tessellationControlShaderModuleName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::setTessellationControlShaderBinding(
            const std::string &tessellationControlShaderModuleName) {
        if (!isValidResourceBindingInput(tessellationControlShaderModuleName,
                                         {shaderBindings->tessellationControlShader})) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        shaderBindings->tessellationControlShader = tessellationControlShaderModuleName;
        return RenderPass::AddBindingResult::SUCCESS;
    }

/**
 *
 * @param geometryShaderModuleName
 * @return
 */
    RenderPass::AddBindingResult PRenderPass::setGeometryShaderBinding(const std::string &geometryShaderModuleName) {
        if (!isValidResourceBindingInput(geometryShaderModuleName, {shaderBindings->geometryShader})) {
            return RenderPass::AddBindingResult::FAILURE;
        }

        shaderBindings->geometryShader = geometryShaderModuleName;
        return RenderPass::AddBindingResult::SUCCESS;
    }

    bool PRenderPass::isValidResourceBindingInput(const std::string &resourceBinding,
                                                  const std::vector<std::string> &existingBindings) {
        return !resourceBinding.empty() &&
               std::all_of(existingBindings.begin(), existingBindings.end(), [&](const std::string &existingBinding) {
                   return resourceBinding != existingBinding;
               });
    }

}// namespace PGraphics