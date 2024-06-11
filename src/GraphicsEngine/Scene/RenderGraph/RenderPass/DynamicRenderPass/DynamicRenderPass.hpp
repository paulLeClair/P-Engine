//
// Created by paull on 2024-05-31.
//

#pragma once

#include "../RenderPass.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {
    class DynamicRenderPass : public RenderPass {
    public:
        struct CreationInput : RenderPass::CreationInput {

        };

        std::shared_ptr<gir::GraphicsIntermediateRepresentation>
        bakeToGIR(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneResourceGirs,
                  const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneShaderModuleGirs) override {
            const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs
                    = obtainBufferGirsFromListOfAllResourceGirs(sceneResourceGirs);

            const std::vector<std::shared_ptr<gir::ImageIR> > &sceneImageGirs
                    = obtainImageGirsFromListOfAllResourceGirs(sceneResourceGirs);

            const std::vector<std::shared_ptr<gir::ShaderConstantIR> > &sceneShaderConstantGirs
                    = obtainShaderConstantGirsFromListOfAllResourceGirs(sceneResourceGirs);

            std::vector<std::shared_ptr<gir::ShaderModuleIR> > sceneShaderModules
                    = obtainShaderModuleGirsList(sceneShaderModuleGirs);

            return std::make_shared<DynamicRenderPassIR>(
                    DynamicRenderPassIR::CreationInput{
                            name,
                            uid,
                            gir::GIRSubtype::RENDER_PASS,
                            RenderPassIR::RenderPassSubtype::DYNAMIC,
                            // geometry bindings
                            bakeRenderPassDrawCommands(drawBindings,
                                                       name,
                                                       sceneBufferGirs),
                            // image bindings
                            bakeListOfSceneImageAttachments(colorAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(inputAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(depthStencilAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(depthOnlyAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(stencilOnlyAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(storageAttachments, sceneImageGirs),
                            bakeListOfSceneImageAttachments(transferSources, sceneImageGirs),
                            bakeListOfSceneImageAttachments(transferDestinations, sceneImageGirs),
                            // texture bindings
                            bakeTextureAttachments(textureAttachments, sceneImageGirs),
                            // shader constant bindings
                            bakeShaderConstantAttachments(shaderConstants, sceneShaderConstantGirs),
                            // buffer bindings
                            bakeUniformBufferAttachments(uniformBuffers, sceneBufferGirs),
                            bakeStorageBufferAttachments(storageBuffers, sceneBufferGirs),
                            bakeStorageTexelBufferAttachments(storageTexelBuffers, sceneBufferGirs),
                            bakeTexelBufferAttachments(texelBuffers, sceneBufferGirs),
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
                            bakeVertexInputState(graphicsPipelineConfiguration.vertexInputConfiguration)
                    });
        }

    };
}


