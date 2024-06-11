//
// Created by paull on 2022-11-15.
//

#pragma once

#include <functional>

#include "../RenderPass.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {

    /**
     * Time to implement this motherfricker!
     *
     *
     */
    class ImguiRenderPass : public RenderPass {
    public:
        struct CreationInput : public RenderPass::CreationInput {
            std::vector<std::function<void()>> initialGuiElementCallbacks;
        };

        explicit ImguiRenderPass(const CreationInput &creationInput)
                : RenderPass(creationInput),
                  guiElementCallbacks(creationInput.initialGuiElementCallbacks) {

        }

        void addGuiElementCallback(std::function<void()> &guiElementCallback) {
            guiElementCallbacks.push_back(guiElementCallback);
        }

        [[nodiscard]] const std::vector<std::function<void()>> &getGuiElementCallbacks() const {
            return guiElementCallbacks;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation>
        bakeToGIR(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneResourceGirs,
                  const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneShaderModuleGirs) override {
            // TODO - make this work so that the dearimgui passes aren't having to look at any shaders
            std::vector<std::shared_ptr<gir::ShaderModuleIR> > sceneShaderModules
                    = obtainShaderModuleGirsList(sceneShaderModuleGirs);

            return std::make_shared<gir::renderPass::dearImgui::DearImguiRenderPassIR>(
                    gir::renderPass::dearImgui::DearImguiRenderPassIR::CreationInput{
                            name,
                            uid,
                            gir::GIRSubtype::RENDER_PASS,
                            RenderPassIR::RenderPassSubtype::DEAR_IMGUI,
                            // geometry bindings
                            {},
                            // image bindings
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            {},
                            // texture bindings
                            {},
                            // shader constant bindings
                            {},
                            // buffer bindings
                            {},
                            {},
                            {},
                            {},
                            // shader bindings
                            bakeVertexShaderModule(vertexShaderAttachment, sceneShaderModules),
                            bakeGeometryShaderModule(geometryShaderAttachment, sceneShaderModules),
                            bakeTessellationControlShaderModule(tessellationControlShaderAttachment,
                                                                sceneShaderModules),
                            bakeTessellationEvaluationShaderModule(tessellationEvaluationShaderAttachment,
                                                                   sceneShaderModules),
                            bakeFragmentShaderModule(fragmentShaderAttachment, sceneShaderModules),
                            // pipeline configuration information
                            bakeColorBlendState(graphicsPipelineConfiguration.colorBlendConfiguration),
                            bakeDepthStencilState(graphicsPipelineConfiguration.depthStencilConfiguration),
                            bakeDynamicState(graphicsPipelineConfiguration.dynamicStateConfiguration),
                            bakeMultisampleState(graphicsPipelineConfiguration.multisampleConfiguration),
                            bakePrimitiveAssemblyState(graphicsPipelineConfiguration.primitiveAssemblyConfiguration),
                            bakeRasterizationState(graphicsPipelineConfiguration.rasterizationConfiguration),
                            bakeTessellationState(graphicsPipelineConfiguration.tessellationConfiguration),
                            bakeVertexInputState(graphicsPipelineConfiguration.vertexInputConfiguration),
                            false,
                            guiElementCallbacks,
                    }
            );
        }


    private:
        std::string name;

        std::vector<std::function<void()>> guiElementCallbacks = {};


    };

}

