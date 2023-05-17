#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../RenderPass.hpp"

namespace PGraphics {

// should not be used by anything but the SceneRenderGraph
    class PRenderPass : public RenderPass {
    public:
        struct CreationInput {
            std::string name;
            ResourceBindings initialResourceBindings = {};
            ShaderBindings initialShaderBindings = {};

            explicit CreationInput(std::string name) : name(std::move(name)) {

            }

            CreationInput(std::string name, ResourceBindings initialResourceBindings,
                          ShaderBindings initialShaderBindings)
                    : name(std::move(name)),
                      initialResourceBindings(std::move(initialResourceBindings)),
                      initialShaderBindings(std::move(initialShaderBindings)) {

            }
        };

        explicit PRenderPass(const CreationInput &createInfo) : name(createInfo.name),
                                                                resourceBindings(std::make_unique<ResourceBindings>(
                                                                        createInfo.initialResourceBindings)),
                                                                shaderBindings(std::make_unique<ShaderBindings>(
                                                                        createInfo.initialShaderBindings)) {
            if (name.empty()) {
                throw std::runtime_error("No name provided for render pass!");
            }
        }

        ~PRenderPass() = default;

        /* SceneResource Bindings Interface */
        AddBindingResult addColorOutputBinding(const std::string &colorOutputImageName) override;

        AddBindingResult addDepthStencilInputBinding(const std::string &colorDepthStencilInputImageName) override;

        AddBindingResult addDepthStencilOutputBinding(const std::string &colorDepthStencilOutputImageName) override;

        AddBindingResult addStorageImageInputBinding(const std::string &storageImageInputImageName) override;

        AddBindingResult addStorageImageOutputBinding(const std::string &storageImageOutputImageName) override;

        AddBindingResult addUniformBufferInputBinding(const std::string &uniformBufferInputImageName) override;

        AddBindingResult
        addUniformTexelBufferInputBinding(const std::string &uniformTexelBufferInputImageName) override;

        AddBindingResult addStorageBufferInputBinding(const std::string &storageBufferInputBufferName) override;

        AddBindingResult addStorageBufferOutputBinding(const std::string &storageBufferOutputBufferName) override;

        AddBindingResult
        addStorageTexelBufferInputBinding(const std::string &storageTexelBufferInputBufferName) override;

        AddBindingResult
        addStorageTexelBufferOutputBinding(const std::string &storageTexelBufferOutputBufferName) override;

        /* Shader Bindings Interface */
        AddBindingResult setVertexShaderBinding(const std::string &vertexShaderModuleName) override;

        AddBindingResult setFragmentShaderBinding(const std::string &fragmentShaderModuleName) override;

        AddBindingResult
        setTessellationEvaluationShaderBinding(const std::string &tessellationEvaluationShaderModuleName) override;

        AddBindingResult
        setTessellationControlShaderBinding(const std::string &tessellationControlShaderModuleName) override;

        AddBindingResult setGeometryShaderBinding(const std::string &geometryShaderModuleName) override;

        [[nodiscard]] const std::string &getName() const override {
            return name;
        }

        [[nodiscard]] const ResourceBindings &getResourceBindings() const override {
            return *resourceBindings;
        }

        [[nodiscard]] const ShaderBindings &getShaderBindings() const override {
            return *shaderBindings;
        }

    private:
        const std::string name;
        std::unique_ptr<ResourceBindings> resourceBindings;
        std::unique_ptr<ShaderBindings> shaderBindings;

        [[nodiscard]] static bool isValidResourceBindingInput(const std::string &resourceBinding,
                                                              const std::vector<std::string> &existingBindings);
    };

}// namespace PGraphics
