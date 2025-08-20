//
// Created by paull on 2024-05-31.
//

#pragma once

#include "RenderPassCore.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {
    /* DYNAMIC DATA -> is updated each frame in an ordering determined by user-specified priority values */
    struct DynamicResourceBinding {
        UniqueIdentifier resourceId = {};

        std::function<std::vector<uint8_t>()> updateCallback = {};

        float priority = 1.0f;
    };

    struct DynamicRenderPass final : RenderPassCore {
        /* STATIC DATA -> doesn't need to be updated after initialization, exists until scene is closed */
        // in the future maybe I'll add something specific here, but for now we can just have any data that isn't
        // specifically marked as dynamic be automatically lumped into the static grouping
        std::vector<UniqueIdentifier> staticResources = {};

        // TODO -> any other resource types that we want to be dynamic
        std::vector<DynamicResourceBinding> dynamicBuffers = {};
        std::vector<DynamicResourceBinding> dynamicImages = {};


        [[nodiscard]] DynamicRenderPassIR
        bakeToGIR(std::vector<gir::BufferIR> sceneBufferGirs,
                  std::vector<gir::ImageIR> sceneImageGirs,
                  std::vector<gir::ShaderConstantIR> sceneShaderConstantGirs,
                  std::vector<gir::model::ModelIR> modelGirs,
                  const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) const {
            // TODO -> validate that the core is set up properly etc


            std::vector<gir::vertex::VertexInputBindingIR> inputBindingGirs(geometryBindings.size());
            unsigned bindingIndex = 0;
            for (auto &geometryBinding: geometryBindings) {
                std::vector<gir::vertex::VertexAttributeIR> attributes(geometryBinding.vertexBinding.attributes.size());
                unsigned attribIndex = 0;
                for (auto &sceneAttrib: geometryBinding.vertexBinding.attributes) {
                    if (!sceneAttrib.usageLabel.has_value()) {
                        // TODO -> log that this scenario happened! This will mess things up
                        break;
                    }

                    attributes[attribIndex] = gir::vertex::VertexAttributeIR{
                        sceneAttrib.attributeName,
                        bindingIndex,
                        sceneAttrib.shaderResourceLocation,
                        (gir::resource::FormatIR) sceneAttrib.attributeFormat,
                        sceneAttrib.vertexOffset,
                        (gir::vertex::VertexAttributeIR::AttributeUsage) static_cast<unsigned>(sceneAttrib.usageLabel.
                            get())
                    };

                    attribIndex++;
                }

                inputBindingGirs[bindingIndex] = gir::vertex::VertexInputBindingIR{
                    gir::vertex::VertexInputBindingIR::CreationInput{
                        name + "Binding Index " + std::to_string(bindingIndex),
                        UniqueIdentifier(),
                        gir::GIRSubtype::VERTEX_INPUT_BINDING,
                        attributes
                    }
                };

                bindingIndex++;
            }

            std::vector<ImageAttachment> aggregatedImageAttachments = {};
            for (auto &colorAttachment: colorAttachments) {
                aggregatedImageAttachments.push_back(colorAttachment);
            }

            gir::renderPass::ImageAttachment depthImageAttachment = {};
            if (!depthAttachment.imageIdentifier.getValue().is_nil())
                depthImageAttachment = gir::renderPass::ImageAttachment{
                    .attachedImage = depthAttachment.imageIdentifier,
                    .imageFormat = (gir::resource::FormatIR) depthAttachment.imageFormat,
                    // TODO -> make sure these defaults are fine
                };

            // we need to be passing in the DynamicRenderPassPipelineStateIR here
            std::vector<DynamicRenderPassPipelineStateIR::UidFormatPair> colorImageAttachments = {};
            for (auto &colorAttachment: aggregatedImageAttachments) {
                colorImageAttachments.emplace_back(colorAttachment.imageIdentifier,
                                                   (gir::resource::FormatIR) colorAttachment.imageFormat);
            }

            const DynamicRenderPassPipelineStateIR pipelineStateGir{
                .colorAttachmentFormats = colorImageAttachments,
                .depthStencilAttachmentFormat = {depthImageAttachment.attachedImage, depthImageAttachment.imageFormat},
                .viewMask = 0
            };

            std::vector<DynamicRenderPassIR::DynamicResourceBinding> dynamicResources = {};
            for (const auto &[resourceId, updateCallback, priority]: dynamicBuffers) {
                dynamicResources.emplace_back(
                    resourceId,
                    updateCallback,
                    priority
                );
            }

            return DynamicRenderPassIR(
                DynamicRenderPassIR::CreationInput{
                    name,
                    uid,
                    gir::GIRSubtype::RENDER_PASS,
                    RenderPassIR::RenderPassSubtype::DYNAMIC,
                    inputBindingGirs,
                    // note: models are now the "rendering primitive" for the single-animated-model demo,
                    // in that they contain the "bound geometry" for the entire render pass (obviously temporary)
                    modelGirs,
                    // "model-less" bound geometries (disabled for the "single-animated-model" demo)
                    {},
                    // image bindings
                    bakeListOfSceneImageAttachments(
                        aggregatedImageAttachments, sceneImageGirs),
                    // texture bindings
                    bakeTextureAttachments(textureAttachments),
                    // shader constant bindings
                    bakeShaderConstantAttachments(shaderConstants, sceneShaderConstantGirs),
                    // buffer bindings
                    bakeUniformBufferAttachments(uniformBuffers),
                    // shader bindings
                    findVertexShaderModule(sceneShaderModuleGirs),
                    findFragmentShaderModule(sceneShaderModuleGirs),
                    // pipeline configuration information
                    bakeColorBlendState(graphicsPipelineConfiguration.colorBlendConfiguration),
                    bakeDepthStencilState(graphicsPipelineConfiguration.depthStencilConfiguration),
                    bakeDynamicState(graphicsPipelineConfiguration.dynamicStateConfiguration),
                    bakeMultisampleState(graphicsPipelineConfiguration.multisampleConfiguration),
                    bakePrimitiveAssemblyState(
                        graphicsPipelineConfiguration.primitiveAssemblyConfiguration),
                    bakeRasterizationState(
                        graphicsPipelineConfiguration.rasterizationConfiguration),
                    bakeTessellationState(graphicsPipelineConfiguration.tessellationConfiguration),
                    bakeVertexInputState(
                        graphicsPipelineConfiguration.vertexInputConfiguration),
                    false,
                    depthImageAttachment,
                    depthImageAttachment.imageFormat, // note: this is redundant lol
                    depthTestEnabled,
                    depthWriteEnabled,
                    pipelineStateGir,
                    {}, // static resources unused rn
                    dynamicResources,
                    {} // dynamic images unused rn
                });
        }
    };
}


