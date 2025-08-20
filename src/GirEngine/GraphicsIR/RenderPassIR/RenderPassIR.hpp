//
// Created by paull on 2023-10-06.
//

#pragma once

#include <memory>
#include "../GraphicsIntermediateRepresentation.hpp"
#include "../GraphicsPipelineIR/GraphicsPipelineIR.hpp"
#include "../DrawAttachmentIR/DrawAttachmentIR.hpp"
#include "../model/ModelIR/ModelIR.hpp"
#include "../ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIR.hpp"

#include "../ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"
#include "ResourceAttachments/BufferAttachment/BufferAttachment.hpp"
#include "ResourceAttachments/ImageAttachment/ImageAttachment.hpp"
#include "ResourceAttachments/TextureAttachment/TextureAttachment.hpp"
#include "ResourceAttachments/ShaderConstantAttachment/ShaderConstantAttachment.hpp"
#include "../VertexInputBindingIR/VertexInputBindingIR.hpp"


namespace pEngine::girEngine::gir::renderPass {
    /**
     * Coming back to this - we seem to be running into problems where we have invalid pointers to shader modules
     * that are created from these attachments (or maybe it's somewhere else)
     */
    struct ShaderGirAttachment {
        // for now I'll just include a pointer to the given shaderIR in the attachment;
        // this is slightly gross because it's a raw pointer; a redesign
        // of how shader attachments are specified can probably avoid this somehow
        const SpirVShaderModuleIR *attachedShaderModuleGir = nullptr;
    };

    class RenderPassIR : public GraphicsIntermediateRepresentation {
    public:
        enum class RenderPassSubtype {
            UNKNOWN,
            DYNAMIC,
            DEAR_IMGUI,
            STATIC
        };


        /**
         * This should be a fairly meaty thing I believe, assuming there's a lot of
         * render pass state that is shared across render passes. \n\n
         *
         * I think it makes sense to have all the graphics pipeline creation stuff be in here...
         * But I think it also makes sense to have it in the new RenderGraphIR class so maybe
         * eventually I'll move it all there.\n\n
         *
         * Granted, to be able to share graphics pipelines across passes we'd need to know each
         * pass' particular configuration anyway, since only then can we know if two passes are compatible.
         * so maybe not if we have to store all this stuff per-render pass anyway. \n\n
         *
         * Now that I think about it, I think the backend should be able to do all that configuration-comparison anyway
         * and leverage graphics pipeline sharing; the scene bake shouldn't concern itself with that probably
         */
        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            RenderPassSubtype subtype = RenderPassSubtype::UNKNOWN;

            // geometry bindings
            std::vector<vertex::VertexInputBindingIR> vertexInputBindings = {};

            // these currently are just a draw attachment with some additional data/capabilities
            // (ie you can have an associated animation etc)
            std::vector<model::ModelIR> models;

            // note: model-less draw attachments are not for the single-animated-model demo
            std::vector<DrawAttachmentIR> drawAttachments = {};

            // image bindings
            std::vector<ImageAttachment> colorAttachments;

            std::vector<TextureAttachment> textureAttachments;

            // shader constant bindings
            std::vector<ShaderConstantIR> shaderConstants;

            // buffer bindings
            std::vector<BufferAttachment> uniformBuffers;

            // shader bindings (hardcoded for spir-v for now)
            const SpirVShaderModuleIR vertexShader;
            const SpirVShaderModuleIR fragmentShader;

            // pipeline configuration information
            ColorBlendStateIR colorBlendState;
            DepthStencilStateIR depthStencilState;
            DynamicStateIR dynamicState;
            MultisampleStateIR multisampleState;
            PrimitiveAssemblyIR primitiveAssemblyState;
            RasterizationStateIR rasterizationState;
            TessellationStateIR tessellationState;
            VertexInputStateIR vertexInputState;

            bool depthBiasEnabled = false;
            ImageAttachment depthStencilAttachment;
            resource::FormatIR depthStencilFormat;
            bool depthTestEnabled = false;
            bool depthWriteEnabled = false;
        };

        explicit RenderPassIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              renderPassSubtype(creationInput.subtype),
              graphicsPipeline(constructGraphicsPipelineFromCreationInput(creationInput)),
              depthBiasEnabled(creationInput.depthBiasEnabled),
              depthTestEnabled(creationInput.depthTestEnabled),
              depthWriteEnabled(creationInput.depthWriteEnabled),
              vertexInputBindings(creationInput.vertexInputBindings),
              drawAttachments(creationInput.drawAttachments),
              models(creationInput.models),
              colorAttachments(creationInput.colorAttachments),
              textureAttachments(creationInput.textureAttachments),
              uniformBuffers(creationInput.uniformBuffers),
              vertexShader(creationInput.vertexShader),
              fragmentShader(creationInput.fragmentShader),
              depthAttachment(creationInput.depthStencilAttachment),
              depthAttachmentFormat(creationInput.depthStencilFormat) {
        }

        RenderPassSubtype renderPassSubtype = RenderPassSubtype::UNKNOWN;

        /**
         * I'm thinking (at least for the first pass) we can have it work so that
         * each graphics pipeline is tied to one particular render pass; \n\n
         *
         * The idea is that this should be built dynamically from information the user provides
         * in the scene render pass.
         */
        std::shared_ptr<GraphicsPipelineIR> graphicsPipeline;

        // miscellaneous config...
        bool depthBiasEnabled = false;

        // geometry bindings
        std::vector<vertex::VertexInputBindingIR> vertexInputBindings = {};

        std::vector<DrawAttachmentIR> drawAttachments;

        std::vector<model::ModelIR> models;

        // image bindings
        std::vector<ImageAttachment> colorAttachments;

        // texture attachments
        std::vector<TextureAttachment> textureAttachments;

        // buffer bindings
        std::vector<BufferAttachment> uniformBuffers;

        // shader constants
        std::vector<ShaderConstantAttachment> shaderConstants;

        // shader bindings (storing a full copy of it for the time being, and also hardcoding for SPIR-V)
        SpirVShaderModuleIR vertexShader;
        SpirVShaderModuleIR fragmentShader;

        // depth attachment
        ImageAttachment depthAttachment = {};
        resource::FormatIR depthAttachmentFormat = resource::FormatIR::UNDEFINED;
        bool depthTestEnabled;
        bool depthWriteEnabled;

        static std::shared_ptr<GraphicsPipelineIR>
        constructGraphicsPipelineFromCreationInput(const CreationInput &creationInput) {
            return std::make_shared<GraphicsPipelineIR>(GraphicsPipelineIR::CreationInput{
                creationInput.name,
                creationInput.uid,
                GIRSubtype::GRAPHICS_PIPELINE,
                // pass color blending state
                creationInput.colorBlendState,
                // pass in depth stencil state
                creationInput.depthStencilState,
                // pass in dynamic state
                creationInput.dynamicState,
                // pass in multisample state
                creationInput.multisampleState,
                // pass in primitive assembly state
                creationInput.primitiveAssemblyState,
                // pass in rasterization state
                creationInput.rasterizationState,
                // pass in tessellation state
                creationInput.tessellationState,
                // pass in vertex input state - this is TODO
                creationInput.vertexInputState
            });
        }
    };
} // gir
