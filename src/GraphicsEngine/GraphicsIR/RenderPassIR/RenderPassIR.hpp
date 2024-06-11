//
// Created by paull on 2023-10-06.
//

#pragma once

#include <memory>
#include "../GraphicsIntermediateRepresentation.hpp"
#include "../GraphicsPipelineIR/GraphicsPipelineIR.hpp"
#include "../DrawAttachmentIR/DrawAttachmentIR.hpp"
#include "../ResourceIR/BufferIR/BufferIR.hpp"
#include "../ResourceIR/ImageIR/ImageIR.hpp"
#include "../ShaderModuleIR/ShaderModuleIR.hpp"

#include "../ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"
#include "ResourceAttachments/BufferAttachment/BufferAttachment.hpp"
#include "ResourceAttachments/ImageAttachment/ImageAttachment.hpp"
#include "ResourceAttachments/TextureAttachment/TextureAttachment.hpp"
#include "ResourceAttachments/ShaderConstantAttachment/ShaderConstantAttachment.hpp"
#include "ShaderAttachment/ShaderAttachment.hpp"


namespace pEngine::girEngine::gir::renderPass {


    // TODO - probably rename this or indicate that it's specific to IR
    // ALSO TODO - probably redesign this... I have a bunch of dumbass shared pointers to this struct which
    // just itself contains a shared pointer...
    struct ShaderGirAttachment {
        // for now I'll just include a pointer to the given shaderIR in the attachment
        std::shared_ptr<ShaderModuleIR> attachedShaderModuleGir = nullptr;

        explicit ShaderGirAttachment(const std::shared_ptr<ShaderModuleIR> &attachedShaderModuleGir)
                : attachedShaderModuleGir(attachedShaderModuleGir) {}
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
        struct CreationInput : public GraphicsIntermediateRepresentation::CreationInput {
            RenderPassSubtype subtype = RenderPassSubtype::UNKNOWN;

            // geometry bindings
            std::vector<std::shared_ptr<DrawAttachmentIR> > drawAttachments;

            // TODO - remove the shared pointers from the attachments, they don't need them

            // image bindings
            std::vector<std::shared_ptr<ImageAttachment> > colorAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > inputAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > depthStencilAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > depthOnlyAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > stencilOnlyAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > storageAttachments;
            std::vector<std::shared_ptr<ImageAttachment> > transferSources;
            std::vector<std::shared_ptr<ImageAttachment> > transferDestinations;
            std::vector<std::shared_ptr<TextureAttachment> > textureAttachments;

            // shader constant bindings
            std::vector<std::shared_ptr<ShaderConstantIR> > shaderConstants;

            // buffer bindings
            std::vector<std::shared_ptr<BufferAttachment> > uniformBuffers;
            std::vector<std::shared_ptr<BufferAttachment> > storageBuffers;
            std::vector<std::shared_ptr<BufferAttachment> > storageTexelBuffers;
            std::vector<std::shared_ptr<BufferAttachment> > texelBuffers;

            // shader bindings
            std::shared_ptr<ShaderGirAttachment> vertexShader;
            std::shared_ptr<ShaderGirAttachment> geometryShader;
            std::shared_ptr<ShaderGirAttachment> tessellationControlShader;
            std::shared_ptr<ShaderGirAttachment> tessellationEvaluationShader;
            std::shared_ptr<ShaderGirAttachment> fragmentShader;

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
        };

        explicit RenderPassIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  renderPassSubtype(creationInput.subtype),
                  graphicsPipeline(constructGraphicsPipelineFromCreationInput(creationInput)),
                  drawAttachments(creationInput.drawAttachments),
                  colorAttachments(creationInput.colorAttachments),
                  inputAttachments(creationInput.inputAttachments),
                  depthStencilAttachments(creationInput.depthStencilAttachments),
                  depthOnlyAttachments(creationInput.depthOnlyAttachments),
                  stencilOnlyAttachments(creationInput.stencilOnlyAttachments),
                  storageAttachments(creationInput.storageAttachments),
                  transferSources(creationInput.transferSources),
                  transferDestinations(creationInput.transferDestinations),
                  textureAttachments(creationInput.textureAttachments),
                  uniformBuffers(creationInput.uniformBuffers),
                  storageBuffers(creationInput.storageBuffers),
                  storageTexelBuffers(creationInput.storageTexelBuffers),
                  texelBuffers(creationInput.texelBuffers),
                  vertexShader(creationInput.vertexShader),
                  geometryShader(creationInput.geometryShader),
                  tessellationControlShader(creationInput.tessellationControlShader),
                  tessellationEvaluationShader(creationInput.tessellationEvaluationShader),
                  fragmentShader(creationInput.fragmentShader),
                  depthBiasEnabled(creationInput.depthBiasEnabled) {
        }

        [[nodiscard]] const GraphicsPipelineIR &getGraphicsPipeline() const {
            return *graphicsPipeline;
        }

        /**
         * Each DrawAttachmentIR object should contain all geometry and information required to draw that geometry. \n\n
         *
         * My reasoning for lumping them together is that geometry doesn't usually exist without being
         * drawn (or otherwise handed to the pipeline somehow), and likewise draw commands don't really exist without
         * geometry - if that turns out to be a bad call I'll change it though. \n\n
         *
         * It seems like a simple way to do things though and would easily support the same buffers being used in
         * multiple draws, as well as easily supporting different types of draws without making the user
         * provide a callback, stuff like that.
         *
         * @return
         */
        [[nodiscard]] const std::vector<std::shared_ptr<DrawAttachmentIR> > &getDrawAttachments() const {
            return drawAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getColorAttachments() const {
            return colorAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getInputAttachments() const {
            return inputAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getDepthStencilAttachments() const {
            return depthStencilAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getDepthOnlyAttachments() const {
            return depthOnlyAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getStencilOnlyAttachments() const {
            return stencilOnlyAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getStorageAttachments() const {
            return storageAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getTransferSources() const {
            return transferSources;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ImageAttachment> > &getTransferDestinations() const {
            return transferDestinations;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<TextureAttachment> > &getTextureAttachments() const {
            return textureAttachments;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ShaderConstantAttachment> > &getShaderConstants() const {
            return shaderConstants;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferAttachment> > &getUniformBuffers() const {
            return uniformBuffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferAttachment> > &getStorageBuffers() const {
            return storageBuffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferAttachment> > &getStorageTexelBuffers() const {
            return storageTexelBuffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<BufferAttachment> > &getTexelBuffers() const {
            return texelBuffers;
        }

        [[nodiscard]] const std::shared_ptr<ShaderGirAttachment> &getVertexShaderAttachment() const {
            return vertexShader;
        }

        [[nodiscard]] const std::shared_ptr<ShaderGirAttachment> &getGeometryShaderAttachment() const {
            return geometryShader;
        }

        [[nodiscard]] const std::shared_ptr<ShaderGirAttachment> &getTessellationControlShaderAttachment() const {
            return tessellationControlShader;
        }

        [[nodiscard]] const std::shared_ptr<ShaderGirAttachment> &getTessellationEvaluationShaderAttachment() const {
            return tessellationEvaluationShader;
        }

        [[nodiscard]] const std::shared_ptr<ShaderGirAttachment> &getFragmentShaderAttachment() const {
            return fragmentShader;
        }

        [[nodiscard]] RenderPassSubtype getRenderPassSubtype() const {
            return renderPassSubtype;
        }

        [[nodiscard]] bool isDepthBiasEnabled() const {
            return depthBiasEnabled;
        }

    private:
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

        /**
         * Now that I think about it, maybe we can dynamically construct the
         * DrawAttachmentIR based on only the scene geometry resources;
         *
         * However, I think the current IR class design is geared towards
         * directly providing a set of draw command IRs and also probably
         * the vertex/index buffers that are going to be bound to it.
         */
        // geometry bindings
        std::vector<std::shared_ptr<DrawAttachmentIR> > drawAttachments;

        // image bindings
        std::vector<std::shared_ptr<ImageAttachment> > colorAttachments;
        std::vector<std::shared_ptr<ImageAttachment> > inputAttachments;
        std::vector<std::shared_ptr<ImageAttachment> > depthStencilAttachments;
        std::vector<std::shared_ptr<ImageAttachment> > depthOnlyAttachments;
        // TODO - merge these 3 depth/stencil lists into one
        std::vector<std::shared_ptr<ImageAttachment> > stencilOnlyAttachments;
        std::vector<std::shared_ptr<ImageAttachment> > storageAttachments;
        std::vector<std::shared_ptr<ImageAttachment> > transferSources;
        std::vector<std::shared_ptr<ImageAttachment> > transferDestinations;

        // texture attachments
        std::vector<std::shared_ptr<TextureAttachment> > textureAttachments;

        // buffer bindings
        std::vector<std::shared_ptr<BufferAttachment> > uniformBuffers;
        std::vector<std::shared_ptr<BufferAttachment> > storageBuffers;
        std::vector<std::shared_ptr<BufferAttachment> > storageTexelBuffers;
        std::vector<std::shared_ptr<BufferAttachment> > texelBuffers;

        // shader constants
        std::vector<std::shared_ptr<ShaderConstantAttachment> > shaderConstants;

        // shader bindings
        std::shared_ptr<ShaderGirAttachment> vertexShader;
        std::shared_ptr<ShaderGirAttachment> geometryShader;
        std::shared_ptr<ShaderGirAttachment> tessellationControlShader;
        std::shared_ptr<ShaderGirAttachment> tessellationEvaluationShader;
        std::shared_ptr<ShaderGirAttachment> fragmentShader;

        static std::shared_ptr<GraphicsPipelineIR>
        constructGraphicsPipelineFromCreationInput(const CreationInput &creationInput) {
            return std::make_shared<GraphicsPipelineIR>(GraphicsPipelineIR::CreationInput{
                    creationInput.name,
                    creationInput.uid,
                    GIRSubtype::GRAPHICS_PIPELINE,
                    // pass color blending state
                    creationInput.colorBlendState.enableColorBlend,
                    creationInput.colorBlendState.enableColorBlendLogicOperation,
                    creationInput.colorBlendState.logicOp,
                    creationInput.colorBlendState.blendConstants,
                    creationInput.colorBlendState.colorAttachmentBlendStates,
                    // pass in depth stencil state
                    creationInput.depthStencilState.enableDepthTesting,
                    creationInput.depthStencilState.enableDepthWrites,
                    creationInput.depthStencilState.depthTestingOperation,
                    creationInput.depthStencilState.enableDepthBoundsTest,
                    creationInput.depthStencilState.depthBoundMinimumValue,
                    creationInput.depthStencilState.depthBoundMaximumValue,
                    creationInput.depthStencilState.enableStencilTest,
                    creationInput.depthStencilState.frontFacingStencilTestState,
                    creationInput.depthStencilState.backFacingStencilTestState,
                    // pass in dynamic state
                    creationInput.dynamicState.dynamicStateEnabled,
                    creationInput.dynamicState.dynamicStateSources,
                    // pass in multisample state
                    creationInput.multisampleState.enableMultisampling,
                    creationInput.multisampleState.minimumNumberOfRasterizationSamples,
                    creationInput.multisampleState.enableSampleShading,
                    creationInput.multisampleState.fractionOfSamplesToShade,
                    creationInput.multisampleState.sampleMask,
                    creationInput.multisampleState.enableAlphaToCoverage,
                    creationInput.multisampleState.enableAlphaToOne,
                    // pass in primitive assembly state
                    creationInput.primitiveAssemblyState.enablePrimitiveRestartForIndexedDraws,
                    creationInput.primitiveAssemblyState.enabledPrimitiveTopologies,
                    // pass in rasterization state
                    creationInput.rasterizationState.enableDepthClamping,
                    creationInput.rasterizationState.enableExtendedDepthClipState,
                    creationInput.rasterizationState.extendedDepthClipState,
                    creationInput.rasterizationState.discardAllPrimitivesBeforeRasterization,
                    creationInput.rasterizationState.rasterizationMode,
                    creationInput.rasterizationState.cullingMode,
                    creationInput.rasterizationState.polygonFrontFaceOrientation,
                    creationInput.rasterizationState.lineWidth,
                    creationInput.rasterizationState.enableDepthBias,
                    creationInput.rasterizationState.depthBiasAdditiveConstantValue,
                    creationInput.rasterizationState.depthBiasClamp,
                    creationInput.rasterizationState.depthBiasSlopeFactor,
                    // pass in tessellation state
                    creationInput.tessellationState.enableTessellation,
                    creationInput.tessellationState.numberOfPatchControlPoints,
                    // pass in vertex input state - this is TODO
                    creationInput.vertexInputState.enableManualSpecification
            });
        }
    };
} // gir
