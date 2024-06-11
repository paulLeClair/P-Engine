#pragma once

#include "ImageAttachment/ImageAttachment.hpp"
#include "DrawAttachment/DrawAttachment.hpp"
#include "ShaderAttachment/ShaderAttachment.hpp"
#include "ShaderConstantAttachment/ShaderConstantAttachment.hpp"
#include "TextureAttachment/TextureAttachment.hpp"
#include "BufferAttachment/BufferAttachment.hpp"

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../GraphicsIR/RenderPassIR/StaticRenderPassIR/StaticRenderPassIR.hpp"
#include "../../../GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp"
#include "GraphicsPipelineConfiguration/GraphicsPipelineConfiguration.hpp"
#include "../../../GraphicsIR/RenderPassIR/ShaderAttachment/ShaderAttachment.hpp"
#include "../../../GraphicsIR/RenderPassIR/DearImguiRenderPassIR/DearImguiRenderPassIR.hpp"

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <iterator>

using namespace pEngine::girEngine::scene::graph::renderPass;
using namespace pEngine::girEngine::scene::graph::renderPass::pipeline;

using namespace pEngine::girEngine::gir::renderPass;

namespace pEngine::girEngine::scene::graph::renderPass {
    /**
     *
     */
    class RenderPass {
    public:
        enum class Subtype {
            DYNAMIC_RENDER_PASS,
            STATIC_RENDER_PASS,
            DEAR_IMGUI_RENDER_PASS
        };

        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
            Subtype subtype;
            pipeline::GraphicsPipelineConfiguration graphicsPipelineConfiguration;

            // TODO - add ability to initialize various attachments in creation input
        };

        explicit RenderPass(const CreationInput &creationInput)
                : name(creationInput.name),
                  uid(creationInput.uid),
                  subtype(creationInput.subtype),
                  graphicsPipelineConfiguration(creationInput.graphicsPipelineConfiguration) {
        }

        ~RenderPass() = default;

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] util::UniqueIdentifier getUniqueIdentifier() const {
            return uid;
        }

        /**
        * This attaches the argument as a color input, where per-pixel values can be read in;
        * Note that it only has certain uses and cannot read any other pixels the way
        * samplers can.
        */
        bool addInputAttachment(const ImageAttachment &attachment) {
            // TODO - probably add some sort of validation
            inputAttachments.push_back(attachment);
            return true;
        }

        /**
         * Attaches the argument as a render target.
         *
         * @param attachment
         * @return
         */
        bool addColorAttachment(const ImageAttachment &attachment) {
            colorAttachments.push_back(attachment);
            return true;
        }

        /**
         * Attach the argument as a depth and stencil attachment.
         *
         * @param attachment
         * @return
         */
        bool addDepthStencilAttachment(const ImageAttachment &attachment) {
            depthStencilAttachments.push_back(attachment);
            return true;
        }

        /**
         * Attach the argument as a depth-only attachment, in case you specifically
         * don't want stencil capabilities.
         *
         * @param attachment
         * @return
         */
        bool addDepthAttachment(const ImageAttachment &attachment) {
            depthOnlyAttachments.push_back(attachment);
            return true;
        }

        /**
         * Attach the argument as a stencil-only attachment, in case you specifically
         * don't want depth buffer capabilities.
         *
         * @param attachment
         * @return
         */
        bool addStencilAttachment(const ImageAttachment &attachment) {
            stencilOnlyAttachments.push_back(attachment);
            return true;
        }

        /**
         * Attach the argument as a storage image, which can be read and written to
         * inside the shaders.
         *
         * @param attachment
         * @return
         */
        bool addStorageImageAttachment(const ImageAttachment &attachment) {
            storageAttachments.push_back(attachment);
            return true;
        }

        /**
         *
         * @param texture
         * @return
         */
        bool addTextureAttachment(const TextureAttachment &texture) {
            textureAttachments.push_back(texture);
            return true;
        }

        /**
         *
         * @param buffer
         * @return
         */
        bool addUniformBufferAttachment(const BufferAttachment &buffer) {
            uniformBuffers.push_back(buffer);
            return true;
        }

        /**
         *
         * @param texelBuffer
         * @return
         */
        bool addTexelBufferAttachment(const BufferAttachment &texelBuffer) {
            texelBuffers.push_back(texelBuffer);
            return true;
        }

        /**
         *
         * @param storageBuffer
         * @return
         */
        bool addStorageBufferAttachment(const BufferAttachment &storageBuffer) {
            storageBuffers.push_back(storageBuffer);
            return true;
        }

        /**
         *
         * @param storageTexelBuffer
         * @return
         */
        bool addStorageTexelBufferAttachment(const BufferAttachment &storageTexelBuffer) {
            storageTexelBuffers.push_back(storageTexelBuffer);
            return true;
        }

        /**
         *
         * @param geometryAttachment
         * @return
         */
        bool addGeometryAttachment(const DrawAttachment &geometryAttachment) {
            drawBindings.push_back(geometryAttachment);
            return true;
        }

        /**
         *
         * @param shaderConstantAttachment
         * @return
         */
        bool addShaderConstantAttachment(const ShaderConstantAttachment &shaderConstantAttachment) {
            shaderConstants.push_back(shaderConstantAttachment);
            return true;
        }

        /**
         *
         * @param shaderModule
         * @return
         */
        bool setVertexShaderModule(const ShaderAttachment &shaderModule) {
            vertexShaderAttachment = shaderModule;
            return true;
        }

        /**
         *
         * @param shaderModule
         * @return
         */
        bool setGeometryShaderModule(const ShaderAttachment &shaderModule) {
            geometryShaderAttachment = shaderModule;
            return true;
        }

        /**
         *
         * @param shaderModule
         * @return
         */
        bool setTessellationControlShaderModule(const ShaderAttachment &shaderModule) {
            tessellationControlShaderAttachment = shaderModule;
            return true;
        }

        /**
         *
         * @param shaderModule
         * @return
         */
        bool setTessellationEvaluationShaderModule(const ShaderAttachment &shaderModule) {
            tessellationEvaluationShaderAttachment = shaderModule;
            return true;
        }

        /**
         *
         * @param shaderModule
         * @return
         */
        bool setFragmentShaderModule(const ShaderAttachment &shaderModule) {
            fragmentShaderAttachment = shaderModule;
            return true;
        }

        /**
         * This is all TODO but I'd like to learn about this and add it in the future.
         * @param shaderModule
         * @return
         */
        [[maybe_unused]] bool setMeshShaderModule(const ShaderAttachment &shaderModule) {
            return false;
        }

        // TODO - ray tracing shader and resource bindings

        /**
         *
         * @param sceneResourceGirs
         * @param sceneShaderModuleGirs
         * @return
         */
//        std::shared_ptr<gir::GraphicsIntermediateRepresentation>
//        bakeToGIR(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneResourceGirs,
//                  const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneShaderModuleGirs) {
//            // build a render pass GIR object depending on the renderPassSubtype of this render pass
//            if (subtype == Subtype::STATIC_RENDER_PASS) {
//                return createPointerToStaticRenderPassIR();
//            } else if (subtype == Subtype::DYNAMIC_RENDER_PASS) {
//                return createPointerToDynamicRenderPass(sceneResourceGirs, sceneShaderModuleGirs);
//            } else if (subtype == Subtype::DEAR_IMGUI_RENDER_PASS) {
//                return createPointerToDearImguiRenderPass(sceneResourceGirs,
//                                                          sceneShaderModuleGirs); // not sure if we need to pass this info in
//            }
//
//            // TODO - log!
//            return nullptr;
//        }

        virtual std::shared_ptr<gir::GraphicsIntermediateRepresentation>
        bakeToGIR(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneResourceGirs,
                  const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneShaderModuleGirs) = 0;

    protected:
        std::string name;
        util::UniqueIdentifier uid;
        Subtype subtype;

        // image attachments
        std::vector<ImageAttachment> colorAttachments = {};
        std::vector<ImageAttachment> inputAttachments = {};
        std::vector<ImageAttachment> depthStencilAttachments = {};
        std::vector<ImageAttachment> depthOnlyAttachments = {};
        std::vector<ImageAttachment> stencilOnlyAttachments = {};
        std::vector<ImageAttachment> storageAttachments = {};
        /**
         * Note: these are only attached to the image as a transfer source;
         * for read/write or write only use the transfer destination binding
         * function as well.
         */
        std::vector<ImageAttachment> transferSources = {};

        /**
        * Note: these are only attached to the image as a transfer destination;
        * for read/write or read only use the transfer destination binding
        * function as well.
        */
        std::vector<ImageAttachment> transferDestinations = {};

        // geometry attachments (to be drawn in this render pass)
        std::vector<DrawAttachment> drawBindings = {};

        // buffer attachments
        std::vector<BufferAttachment> indexBuffers = {};
        std::vector<BufferAttachment> vertexBuffers = {};
        std::vector<BufferAttachment> storageBuffers = {};
        std::vector<BufferAttachment> storageTexelBuffers = {};
        std::vector<BufferAttachment> texelBuffers = {};
        std::vector<BufferAttachment> uniformBuffers = {};

        // shader constant attachments
        std::vector<ShaderConstantAttachment> shaderConstants = {};

        // texture attachments
        std::vector<TextureAttachment> textureAttachments = {};

        // traditional pipeline shader attachments
        ShaderAttachment vertexShaderAttachment = {};
        ShaderAttachment geometryShaderAttachment = {};
        ShaderAttachment tessellationControlShaderAttachment = {};
        ShaderAttachment tessellationEvaluationShaderAttachment = {};
        ShaderAttachment fragmentShaderAttachment = {};
        // todo - add other shader attachments (mesh shaders, ray tracing, etc) when that becomes relevant

        GraphicsPipelineConfiguration graphicsPipelineConfiguration;

        /**
         * Given a list of scene resource girs, this bakes one DrawAttachmentIR object for each DrawAttachment
         * in the RenderPass.
         *
         * @param sceneResourceGirs
         * @return a list of DrawAttachmentIR constructed 1-1 with the RenderPass' geometry attachments.
         */
        static std::vector<std::shared_ptr<gir::DrawAttachmentIR> >
        bakeRenderPassDrawCommands(std::vector<DrawAttachment> &renderPassGeometryAttachments,
                                   std::string &renderPassName,
                                   const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            // naive/initial approach: make one DrawAttachmentIR for each geometry binding
            std::vector<std::shared_ptr<gir::DrawAttachmentIR> > renderPassDrawCommandGirs = {};

            for (auto &geometryAttachment: renderPassGeometryAttachments) {
                // since draw commands don't map 1-1 with a single scene object,
                // build the name dynamically (and assign it a new UID)
                std::string drawCommandsName =
                        getDefaultDrawCommandsIRName(renderPassName, renderPassDrawCommandGirs.size());

                // create new DrawAttachmentIR object and add it to the list being returned
                renderPassDrawCommandGirs.push_back(
                        createDrawCommandsGir(geometryAttachment, drawCommandsName, sceneBufferGirs)
                );
            }

            return renderPassDrawCommandGirs;
        }


        /**
         * Bakes a list of render pass texture attachments into GIR texture attachments.
         *
         * @param textureAttachments
         * @param sceneImageGirs
         * @return a gir::renderPass::TextureAttachment vector
         */
        static std::vector<std::shared_ptr<gir::renderPass::TextureAttachment> >
        bakeTextureAttachments(const std::vector<TextureAttachment> &textureAttachments,
                               const std::vector<std::shared_ptr<gir::ImageIR> > &sceneImageGirs) {
            // obtain attached image girs from global list given the list of texture attachments
            std::vector<std::shared_ptr<gir::ImageIR> > attachedTextureImageGirs = {};
            copyAttachedTextureImagesFromGlobalResourceGirList(textureAttachments,
                                                               sceneImageGirs,
                                                               attachedTextureImageGirs);

            // return early if there are no attached textures
            if (attachedTextureImageGirs.empty()) {
                return {};
            }

            // build texture attachment girs
            std::vector<std::shared_ptr<gir::renderPass::TextureAttachment> > textureAttachmentGirs = {};
            for (const auto &textureAttachment: textureAttachments) {
                textureAttachmentGirs.push_back(
                        createTextureAttachment(attachedTextureImageGirs,
                                                textureAttachment,
                                                textureAttachment.textureResource,
                                                textureAttachment.textureResource->getSamplerSettings()
                        )
                );
            }
            return textureAttachmentGirs;
        }

        /**
         *
         * @param shaderConstantAttachments
         * @param shaderConstantGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::ShaderConstantIR> >
        bakeShaderConstantAttachments(const std::vector<ShaderConstantAttachment> &shaderConstantAttachments,
                                      const std::vector<std::shared_ptr<gir::ShaderConstantIR> > &shaderConstantGirs) {
            std::vector<std::shared_ptr<gir::ShaderConstantIR> > shaderConstantAttachmentGirs = {};

            for (auto &renderPassShaderConstantAttachment: shaderConstantAttachmentGirs) {
                // TODO - add shader constant GIR to list... (will return to this during backend bake)
            }

            return shaderConstantAttachmentGirs;
        }

        /**
         *
         * @param uniformBufferAttachments
         * @param sceneBufferGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> >
        bakeUniformBufferAttachments(const std::vector<BufferAttachment> &uniformBufferAttachments,
                                     const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            return bakeListOfBufferAttachments(uniformBufferAttachments, sceneBufferGirs);
        }

        /**
         *
         * @param storageBufferAttachments
         * @param sceneBufferGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> >
        bakeStorageBufferAttachments(const std::vector<BufferAttachment> &storageBufferAttachments,
                                     const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            return bakeListOfBufferAttachments(storageBufferAttachments, sceneBufferGirs);
        }

        /**
         *
         * @param storageTexelBufferAttachments
         * @param sceneBufferGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> >
        bakeStorageTexelBufferAttachments(const std::vector<BufferAttachment> &storageTexelBufferAttachments,
                                          const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            return bakeListOfBufferAttachments(storageTexelBufferAttachments, sceneBufferGirs);
        }

        /**
         *
         * @param texelBufferAttachments
         * @param sceneBufferGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> >
        bakeTexelBufferAttachments(const std::vector<BufferAttachment> &texelBufferAttachments,
                                   const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            return bakeListOfBufferAttachments(texelBufferAttachments, sceneBufferGirs);
        }

        /**
         *
         * @param shaderAttachment
         * @param sceneShaderModuleGirs
         * @return
         */
        static std::shared_ptr<gir::renderPass::ShaderGirAttachment>
        bakeVertexShaderModule(const ShaderAttachment &shaderAttachment,
                               const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &sceneShaderModuleGirs) {
            const auto &vertexShaderModuleGirItr
                    = std::find_if(sceneShaderModuleGirs.begin(),
                                   sceneShaderModuleGirs.end(),
                                   [&](const std::shared_ptr<gir::ShaderModuleIR> &shaderModuleGir) {
                                       return shaderModuleGir->getUsage() ==
                                              gir::ShaderModuleIR::ShaderUsage::VERTEX_SHADER;
                                   });

            if (vertexShaderModuleGirItr == sceneShaderModuleGirs.end()) {
                // TODO - better logging
                throw std::runtime_error("Error in RenderPass::bakeVertexShaderModule() -> no vertex shader found!");
            }

            return bakeShaderAttachment(shaderAttachment, *vertexShaderModuleGirItr);
        }

        /**
         *
         * @param shaderAttachment
         * @param sceneShaderModuleGirs
         * @return
         */
        static std::shared_ptr<gir::renderPass::ShaderGirAttachment>
        bakeGeometryShaderModule(const ShaderAttachment &shaderAttachment,
                                 const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &sceneShaderModuleGirs) {
            const auto &geometryShaderModuleGirItr
                    = std::find_if(sceneShaderModuleGirs.begin(),
                                   sceneShaderModuleGirs.end(),
                                   [&](const std::shared_ptr<gir::ShaderModuleIR> &shaderModuleGir) {
                                       return shaderModuleGir->getUsage() ==
                                              gir::ShaderModuleIR::ShaderUsage::GEOMETRY_SHADER;
                                   });

            if (geometryShaderModuleGirItr == sceneShaderModuleGirs.end()) {
                // TODO - better logging, add sanity checking for when a geometry shader is requested but not found
                return nullptr; // geometry shaders are not strictly necessary
            }

            return bakeShaderAttachment(shaderAttachment, *geometryShaderModuleGirItr);
        }

        /**
         *
         * @param shaderAttachment
         * @param sceneShaderModuleGirs
         * @return
         */
        static std::shared_ptr<gir::renderPass::ShaderGirAttachment>
        bakeTessellationControlShaderModule(const ShaderAttachment &shaderAttachment,
                                            const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &
                                            sceneShaderModuleGirs) {
            const auto &tessControlShaderModuleGirItr
                    = std::find_if(sceneShaderModuleGirs.begin(),
                                   sceneShaderModuleGirs.end(),
                                   [&](const std::shared_ptr<gir::ShaderModuleIR> &shaderModuleGir) {
                                       return shaderModuleGir->getUsage() ==
                                              gir::ShaderModuleIR::ShaderUsage::TESSELLATION_CONTROL_SHADER;
                                   });

            if (tessControlShaderModuleGirItr == sceneShaderModuleGirs.end()) {
                // TODO - better logging, add sanity checking for when a tess control shader is requested but not found
                return nullptr; // tess control shaders are not strictly necessary
            }

            return bakeShaderAttachment(shaderAttachment, *tessControlShaderModuleGirItr);
        }

        /**
         *
         * @param shaderAttachment
         * @param sceneShaderModuleGirs
         * @return
         */
        static std::shared_ptr<ShaderGirAttachment>
        bakeTessellationEvaluationShaderModule(const ShaderAttachment &shaderAttachment,
                                               const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &
                                               sceneShaderModuleGirs) {
            const auto &tessEvaluationShaderModuleGirItr
                    = std::find_if(sceneShaderModuleGirs.begin(),
                                   sceneShaderModuleGirs.end(),
                                   [&](const std::shared_ptr<gir::ShaderModuleIR> &shaderModuleGir) {
                                       return shaderModuleGir->getUsage() ==
                                              gir::ShaderModuleIR::ShaderUsage::TESSELLATION_EVALUATION_SHADER;
                                   });

            if (tessEvaluationShaderModuleGirItr == sceneShaderModuleGirs.end()) {
                // TODO - better logging, add sanity checking for when a tess evaluation shader is requested but not found
                return nullptr; // tess evaluation shaders are not strictly necessary
            }

            return bakeShaderAttachment(shaderAttachment, *tessEvaluationShaderModuleGirItr);
        }

        /**
         *
         * @param shaderAttachment
         * @param sceneShaderModuleGirs
         * @return
         */
        static std::shared_ptr<gir::renderPass::ShaderGirAttachment>
        bakeFragmentShaderModule(const ShaderAttachment &shaderAttachment,
                                 const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &sceneShaderModuleGirs) {
            const auto &fragmentShaderModuleGirItr
                    = std::find_if(sceneShaderModuleGirs.begin(),
                                   sceneShaderModuleGirs.end(),
                                   [&](const std::shared_ptr<gir::ShaderModuleIR> &shaderModuleGir) {
                                       return shaderModuleGir->getUsage() ==
                                              gir::ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER;
                                   });

            if (fragmentShaderModuleGirItr == sceneShaderModuleGirs.end()) {
                // TODO - better logging
                throw std::runtime_error(
                        "Error in RenderPass::bakeFragmentShaderModule() -> no fragment shader found!");
            }

            return bakeShaderAttachment(shaderAttachment, *fragmentShaderModuleGirItr);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::ColorBlendStateIR bakeColorBlendState(const ColorBlendConfiguration &configuration) {
            // for now, some static conversion maps for any enums will probably do... eventually we'll refactor this

            // define logic operation conversion map
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation,
                    gir::pipeline::ColorBlendStateIR::LogicOperation
            > LOGIC_OPERATION_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::NO_OP,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::NO_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::CLEAR,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::CLEAR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::AND,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::AND
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::AND_REVERSE,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::AND_REVERSE
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::COPY,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::COPY
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::AND_INVERTED,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::AND_INVERTED
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::XOR,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::XOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::OR,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::OR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::NOR,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::NOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::EQUIVALENT,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::EQUIVALENT
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::INVERT,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::INVERT
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::OR_REVERSE,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::OR_REVERSE
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::COPY_INVERTED,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::COPY_INVERTED
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::OR_INVERTED,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::OR_INVERTED
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::NAND,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::NAND
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation::SET,
                                    gir::pipeline::ColorBlendStateIR::LogicOperation::SET
                            },
                    };
            // define blend factor conversion map
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor,
                    gir::pipeline::ColorBlendStateIR::BlendFactor
            > BLEND_FACTOR_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ZERO,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ZERO
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::SRC_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::SRC_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::DST_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::DST_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_DST_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_DST_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::SRC_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::SRC_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::DST_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::DST_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_DST_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_DST_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::CONSTANT_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::CONSTANT_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_CONSTANT_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_CONSTANT_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::CONSTANT_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::CONSTANT_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_CONSTANT_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_CONSTANT_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::SRC_ALPHA_SATURATE,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::SRC_ALPHA_SATURATE
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::SRC1_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::SRC1_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC1_COLOR,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC1_COLOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::SRC1_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::SRC1_ALPHA
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC1_ALPHA,
                                    gir::pipeline::ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC1_ALPHA
                            },
                    };
            // define blend operation conversion map
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation,
                    gir::pipeline::ColorBlendStateIR::BlendOperation
            > BLEND_OPERATION_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::NO_OP,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::NO_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::ADD,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::ADD
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::SUBTRACT,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::SUBTRACT
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::REVERSE_SUBTRACT,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::REVERSE_SUBTRACT
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::MIN,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::MIN
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation::MAX,
                                    gir::pipeline::ColorBlendStateIR::BlendOperation::MAX
                            },
                    };
            // define color component conversion map
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent,
                    gir::pipeline::ColorBlendStateIR::ColorComponent
            > COLOR_COMPONENT_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent::RED,
                                    gir::pipeline::ColorBlendStateIR::ColorComponent::RED
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent::GREEN,
                                    gir::pipeline::ColorBlendStateIR::ColorComponent::GREEN
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent::BLUE,
                                    gir::pipeline::ColorBlendStateIR::ColorComponent::BLUE
                            },
                            {
                                    scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent::ALPHA,
                                    gir::pipeline::ColorBlendStateIR::ColorComponent::ALPHA
                            },
                    };

            return createColorBlendStateGir(configuration,
                                            LOGIC_OPERATION_CONVERSION_MAP,
                                            BLEND_FACTOR_CONVERSION_MAP,
                                            BLEND_OPERATION_CONVERSION_MAP,
                                            COLOR_COMPONENT_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::DepthStencilStateIR
        bakeDepthStencilState(const DepthStencilConfiguration &configuration) {
            std::unordered_map<
                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation,
                    gir::pipeline::DepthStencilStateIR::CompareOperation
            > COMPARE_OPERATION_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::NO_OP,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::NO_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::NEVER,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::NEVER
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::LESS,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::LESS
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::EQUAL,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::EQUAL
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::LESS_OR_EQUAL,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::LESS_OR_EQUAL
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::GREATER,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::GREATER
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::NOT_EQUAL,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::NOT_EQUAL
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::GREATER_OR_EQUAL,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::GREATER_OR_EQUAL
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::CompareOperation::ALWAYS,
                                    gir::pipeline::DepthStencilStateIR::CompareOperation::ALWAYS
                            },
                    };
            std::unordered_map<
                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation,
                    gir::pipeline::DepthStencilStateIR::StencilOperation
            > STENCIL_OPERATION_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::NO_OP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::NO_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::KEEP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::KEEP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::ZERO,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::ZERO
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::REPLACE,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::REPLACE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::INCREMENT_AND_CLAMP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::INCREMENT_AND_CLAMP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::DECREMENT_AND_CLAMP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::DECREMENT_AND_CLAMP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::INVERT,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::INVERT
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::INCREMENT_AND_WRAP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::INCREMENT_AND_WRAP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DepthStencilConfiguration::StencilOperation::DECREMENT_AND_WRAP,
                                    gir::pipeline::DepthStencilStateIR::StencilOperation::DECREMENT_AND_WRAP
                            },
                    };

            return createDepthStencilStateGir(configuration,
                                              COMPARE_OPERATION_CONVERSION_MAP,
                                              STENCIL_OPERATION_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::DynamicStateIR bakeDynamicState(const DynamicStateConfiguration &configuration) {
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource,
                    gir::pipeline::DynamicStateIR::DynamicStateSource
            > DYNAMIC_STATE_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VIEWPORT,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_VIEWPORT
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_SCISSOR,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_SCISSOR
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_LINE_WIDTH,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_LINE_WIDTH
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BIAS,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BIAS
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_BLEND_CONSTANTS,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_BLEND_CONSTANTS
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_COMPARE_MASK,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_COMPARE_MASK
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_WRITE_MASK,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_WRITE_MASK
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_REFERENCE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_REFERENCE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_CULL_MODE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_CULL_MODE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_FRONT_FACE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_FRONT_FACE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_PRIMITIVE_TOPOLOGY,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_PRIMITIVE_TOPOLOGY
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VIEWPORT_WITH_COUNT,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_VIEWPORT_WITH_COUNT
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_SCISSOR_WITH_COUNT,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_SCISSOR_WITH_COUNT
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_TEST_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_TEST_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_WRITE_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_WRITE_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_COMPARE_OP,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_COMPARE_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_TEST_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_TEST_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_OP,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_OP
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_RASTERIZER_DISCARD_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_RASTERIZER_DISCARD_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BIAS_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BIAS_ENABLE
                            },
                            {
                                    scene::graph::renderPass::pipeline::DynamicStateConfiguration::DynamicStateSource::DYNAMIC_PRIMITIVE_RESTART_ENABLE,
                                    gir::pipeline::DynamicStateIR::DynamicStateSource::DYNAMIC_PRIMITIVE_RESTART_ENABLE
                            },
                    };

            return createDynamicStateGir(configuration, DYNAMIC_STATE_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::MultisampleStateIR bakeMultisampleState(const MultisampleConfiguration &configuration) {
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples,
                    gir::pipeline::MultisampleStateIR::NumberOfSamples
            > NUMBER_OF_SAMPLES_CONVERSION_MAP
                    = {
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::UNSET,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::UNSET
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::ONE,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::ONE
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::TWO,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::TWO
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::FOUR,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::FOUR
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::EIGHT,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::EIGHT
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::SIXTEEN,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::SIXTEEN
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::THIRTY_TWO,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::THIRTY_TWO
                            },
                            {
                                    scene::graph::renderPass::pipeline::MultisampleConfiguration::NumberOfSamples::SIXTY_FOUR,
                                    gir::pipeline::MultisampleStateIR::NumberOfSamples::SIXTY_FOUR
                            },
                    };

            return createMultisampleStateGir(configuration, NUMBER_OF_SAMPLES_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::PrimitiveAssemblyIR
        bakePrimitiveAssemblyState(const PrimitiveAssemblyConfiguration &configuration) {
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology,
                    gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology
            > PRIMITIVE_TOPOLOGY_CONVERSION_MAP = {
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::POINT_LIST,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::POINT_LIST
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_LIST,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::LINE_LIST
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_STRIP,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::LINE_STRIP
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_LIST,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_LIST
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_STRIP,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_STRIP
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_FAN,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_FAN
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY
                    },
                    {
                            scene::graph::renderPass::pipeline::PrimitiveAssemblyConfiguration::PrimitiveTopology::PATCH_LIST,
                            gir::pipeline::PrimitiveAssemblyIR::PrimitiveTopology::PATCH_LIST
                    },
            };

            return createPrimitiveAssemblyGir(configuration, PRIMITIVE_TOPOLOGY_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::RasterizationStateIR
        bakeRasterizationState(const RasterizationConfiguration &configuration) {
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonRasterizationMode,
                    gir::pipeline::RasterizationStateIR::PolygonRasterizationMode
            > POLYGON_RASTERIZATION_MODE_CONVERSION_MAP = {
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonRasterizationMode::FILL,
                            gir::pipeline::RasterizationStateIR::PolygonRasterizationMode::FILL
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonRasterizationMode::POINTS,
                            gir::pipeline::RasterizationStateIR::PolygonRasterizationMode::POINTS
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonRasterizationMode::LINES,
                            gir::pipeline::RasterizationStateIR::PolygonRasterizationMode::LINES
                    },
            };
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonCullingMode,
                    gir::pipeline::RasterizationStateIR::PolygonCullingMode
            > POLYGON_CULLING_MODE_CONVERSION_MAP = {
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonCullingMode::NONE,
                            gir::pipeline::RasterizationStateIR::PolygonCullingMode::NONE
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonCullingMode::FRONT_FACING,
                            gir::pipeline::RasterizationStateIR::PolygonCullingMode::FRONT_FACING
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonCullingMode::BACK_FACING,
                            gir::pipeline::RasterizationStateIR::PolygonCullingMode::BACK_FACING
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonCullingMode::ALL,
                            gir::pipeline::RasterizationStateIR::PolygonCullingMode::ALL
                    },
            };
            static const std::unordered_map<
                    scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonFrontFaceOrientationMode,
                    gir::pipeline::RasterizationStateIR::PolygonFrontFaceOrientationMode
            > POLYGON_FRONT_FACE_ORIENTATION_CONVERSION_MAP = {
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonFrontFaceOrientationMode::CLOCKWISE,
                            gir::pipeline::RasterizationStateIR::PolygonFrontFaceOrientationMode::CLOCKWISE
                    },
                    {
                            scene::graph::renderPass::pipeline::RasterizationConfiguration::PolygonFrontFaceOrientationMode::COUNTER_CLOCKWISE,
                            gir::pipeline::RasterizationStateIR::PolygonFrontFaceOrientationMode::COUNTER_CLOCKWISE
                    },
            };

            return createRasterizationStateGir(configuration,
                                               POLYGON_RASTERIZATION_MODE_CONVERSION_MAP,
                                               POLYGON_CULLING_MODE_CONVERSION_MAP,
                                               POLYGON_FRONT_FACE_ORIENTATION_CONVERSION_MAP);
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::TessellationStateIR
        bakeTessellationState(const TessellationConfiguration &configuration) {
            return {
                    configuration.enableTessellation,
                    configuration.numberOfPatchControlPoints
            };
        }

        /**
         *
         * @param configuration
         * @return
         */
        static gir::pipeline::VertexInputStateIR bakeVertexInputState(const VertexInputConfiguration &configuration) {
            return {
                    configuration.enableManualSpecification
            };
        }

        // DRAW COMMANDS BAKE HELPER FUNCTIONS

        /**
         *
         * @param renderPassName
         * @param renderPassDrawCommandsIndex
         * @return
         */
        [[nodiscard]] static std::string
        getDefaultDrawCommandsIRName(const std::string &renderPassName, unsigned int renderPassDrawCommandsIndex) {
            return "renderPass" + renderPassName + "DrawAttachmentIR" +
                   std::to_string(renderPassDrawCommandsIndex);
        }

        static std::vector<gir::vertex::VertexAttributeIR>
        obtainAttachedVertexAttributeIRs(std::vector<vertex::VertexAttributeDescription> &vertexAttributes) {
            std::vector<gir::vertex::VertexAttributeIR> attributeGirs = {};
            attributeGirs.reserve(vertexAttributes.size());
            for (const auto &attribute: vertexAttributes) {
                attributeGirs.emplace_back(
                        attribute.attributeName,
                        attribute.attributeShaderLocation,
                        attribute.attributeByteOffset,
                        attribute.componentSignedness,
                        attribute.componentSpace,
                        attribute.componentComposition,
                        attribute.individualComponentSizesInBytes,
                        attribute.dataPacking
                );
            }
            return attributeGirs;
        }

/**
         * This is a helper function for creating a draw commands IR pointer
         * from a geometry attachment (since they map 1-1 with DrawAttachmentIR for now)
         * and a list of all the BufferIR objects for a scene. If a bound BufferIR can't be found,
         * it should probably throw. Cleaning this up is TODO
         * \n\n
         *
         * Warning: sanity checking is TODO for all this stuff!!!
         *
         * @param geometryAttachment    the attachment that we want to build the DrawAttachmentIR from
         * @param drawCommandsName      the name we want to give the new DrawAttachmentIR class
         * @param sceneBufferGirs       a list of all BufferIR objects for our scene
         * @return a DrawAttachmentIR shared pointer created from the given arguments
         */
        static std::shared_ptr<gir::DrawAttachmentIR>
        createDrawCommandsGir(
                DrawAttachment &geometryAttachment,
                const std::string &drawCommandsName,
                const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            // build DrawAttachmentIR dynamically from RenderPass info and arguments
            // TODO - expand possible draw attachments depending on the type of rendering pipeline (mesh, raytracing)
            return std::make_shared<gir::DrawAttachmentIR>(
                    gir::DrawAttachmentIR::CreationInput{
                            drawCommandsName,
                            UniqueIdentifier(), // create new UID for each new DrawAttachmentIR created
                            gir::GIRSubtype::DRAW_COMMANDS,
                            convertDrawTypeFromGeometryAttachmentToGirFormat(geometryAttachment.drawType),
                            obtainAttachedVertexAttributeIRs(geometryAttachment.vertexAttributes),
                            geometryAttachment.vertexAttributeHash,
                            obtainAttachedVertexBufferGirsFromGeometryAttachment(geometryAttachment,
                                                                                 sceneBufferGirs),
                            obtainAttachedIndexBufferGirsFromGeometryAttachment(geometryAttachment,
                                                                                sceneBufferGirs)
                    }
            );
        }

        /**
         *
         * @param attachmentDrawType
         * @return
         */
        static gir::DrawAttachmentIR::DrawType
        convertDrawTypeFromGeometryAttachmentToGirFormat(DrawAttachment::DrawType &attachmentDrawType) {
            // TODO - determine this based off the geometry attachment draw type
            return gir::DrawAttachmentIR::DrawType::INSTANCED_DRAW;
        }

        /**
         * This obtains all vertex buffers that are bound in the given attachment given the overall list of all buffers.
         *
         * @param attachment -> attachment whose buffers will be compared against the list of all buffer GIRs
         * @param bufferGirs -> list of all buffer GIRs
         * @return All vertex buffers bound to the attachment
         */
        static std::vector<std::shared_ptr<gir::BufferIR> >
        obtainAttachedVertexBufferGirsFromGeometryAttachment(
                DrawAttachment &attachment,
                const std::vector<std::shared_ptr<gir::BufferIR> > &bufferGirs) {
            /**
             * Returning to this, I'll have to do a bit of refactoring to make this work again;
             * it seems goofy as fuck to be iterating over all the buffers. Gotta be a better way...
             *
             * on the other hand i think it is the way it is because the buffers are all pre-baked
             */

            // copy pointer if buffer's usage is set to vertex buffer, and it's contained in the geometry attachment
            std::vector<std::shared_ptr<gir::BufferIR> > attachedVertexBufferGirs = {};
            std::copy_if(bufferGirs.begin(), bufferGirs.end(), std::back_inserter(attachedVertexBufferGirs),
                         [&](const std::shared_ptr<gir::BufferIR> &buffer) {
                             return buffer->getUsage() == gir::BufferIR::BufferUsage::VERTEX_BUFFER
                                    && vertexBufferIsContainedInDrawAttachment(buffer, attachment);
                         });
            return attachedVertexBufferGirs;
        }

        /**
         * This obtains all index buffers that are bound in the given attachment given the overall list of all buffers.
         *
         * @param attachment -> attachment whose buffers will be compared against the list of all buffer GIRs
         * @param bufferGirs -> list of all buffer GIRs
         * @return All index buffers bound to the attachment
         */
        static std::vector<std::shared_ptr<gir::BufferIR> >
        obtainAttachedIndexBufferGirsFromGeometryAttachment(
                DrawAttachment &attachment,
                const std::vector<std::shared_ptr<gir::BufferIR> > &bufferGirs) {
            // copy pointer if buffer's usage is set to index buffer, and it's contained in the geometry attachment
            std::vector<std::shared_ptr<gir::BufferIR> > attachedIndexBufferGirs = {};
            std::copy_if(bufferGirs.begin(), bufferGirs.end(), std::back_inserter(attachedIndexBufferGirs),
                         [&](const std::shared_ptr<gir::BufferIR> &buffer) {
                             return buffer->getUsage() == gir::BufferIR::BufferUsage::INDEX_BUFFER
                                    && indexBufferIsContainedInDrawAttachment(buffer, attachment);
                         });
            return attachedIndexBufferGirs;
        }

        /**
         *
         * @param bufferIR
         * @param attachment
         * @return whether a buffer is found inside a geometry attachment
         */
        static bool vertexBufferIsContainedInDrawAttachment(const std::shared_ptr<gir::BufferIR> &bufferIR,
                                                            const DrawAttachment &attachment) {
            // check if the bufferIR corresponds to any buffers within the geometry attachment's vertex buffers
            if (std::any_of(attachment.vertexBuffers.begin(), attachment.vertexBuffers.end(),
                            [&](const std::shared_ptr<Buffer> &buffer) {
                                return buffer->getUid() == bufferIR->getUid();
                            })) {
                return true;
            }
            return false;
        };


        /**
         *
         * @param bufferIR
         * @param attachment
         * @return whether a buffer is found inside a geometry attachment
         */
        static bool indexBufferIsContainedInDrawAttachment(const std::shared_ptr<gir::BufferIR> &bufferIR,
                                                           const DrawAttachment &attachment) {
            // check if the bufferIR corresponds to any buffers within the geometry attachment's vertex buffers
            if (std::any_of(attachment.indexBuffers.begin(), attachment.indexBuffers.end(),
                            [&](const std::shared_ptr<Buffer> &buffer) {
                                return buffer->getUid() == bufferIR->getUid();
                            })) {
                return true;
            }
            return false;
        };

        /**
         *
         * @param bufferIRToTest
         * @param model
         * @return
         */
        static bool
        isBufferBoundToModel(const std::shared_ptr<gir::BufferIR> &bufferIRToTest,
                             const std::shared_ptr<Model> &model) {
            if (std::any_of(model->getBuffers().begin(), model->getBuffers().end(),
                            [&](const std::shared_ptr<Buffer> &buffer) {
                                return buffer->getUid() == bufferIRToTest->getUid();
                            })) {
                return true;
            }

            return std::any_of(model->getRenderables().begin(), model->getRenderables().end(),
                               [&](const std::shared_ptr<Renderable> &renderable) {
                                   return isBufferBoundToRenderable(bufferIRToTest, renderable);
                               });
        }

        /**
         *
         * @param bufferIRToTest
         * @param renderable
         * @return
         */
        static bool
        isBufferBoundToRenderable(const std::shared_ptr<gir::BufferIR> &bufferIRToTest,
                                  const std::shared_ptr<Renderable> &renderable) {
            // for now, renderables just batch up vertex & index buffers; this will have to be extended in the future

            if (std::any_of(renderable->getVertexBuffers().begin(), renderable->getVertexBuffers().end(),
                            [&](const std::shared_ptr<Buffer> &buffer) {
                                return buffer->getUid() == bufferIRToTest->getUid();
                            })) {
                return true;
            }

            return std::any_of(renderable->getIndexBuffers().begin(), renderable->getIndexBuffers().end(),
                               [&](const std::shared_ptr<Buffer> &buffer) {
                                   return buffer->getUid() == bufferIRToTest->getUid();
                               });
        }

        /**
         * Given a list of scene resource GIR objects, this returns all BufferIRs within it.
         * @param sceneResourceGirs
         * @return The buffers
         */
        [[nodiscard]] static std::vector<std::shared_ptr<gir::BufferIR> >
        obtainBufferGirsFromListOfAllResourceGirs(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneResourceGirs) {
            // copy only the buffer resources into the bufferGirs list
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > bufferGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(bufferGirs),
                         [&](const std::shared_ptr<gir::GraphicsIntermediateRepresentation> &sceneResource) {
                             // copy element if its renderPassSubtype denotes it as a buffer
                             return sceneResource->getSubtype() == gir::GIRSubtype::BUFFER;
                         }
            );
            // cast the bufferGirs to their actual pointer type (BufferIR)
            std::vector<std::shared_ptr<gir::BufferIR> > convertedBufferGirs = {};
            std::transform(bufferGirs.begin(), bufferGirs.end(), std::back_inserter(convertedBufferGirs),
                           [&](std::shared_ptr<gir::GraphicsIntermediateRepresentation> &gir) {
                               // cast the shared pointer to BufferIR
                               return std::dynamic_pointer_cast<gir::BufferIR>(gir);
                           });
            return convertedBufferGirs;
        }

        // IMAGE BAKE HELPER FUNCTIONS

        /**
         * This is a helper function that takes a list of ImageAttachments and
         * bakes them into the corresponding list of ImageIR classes
         *
         * OKAY so now that we're changing the design so that render pass girs are gonna
         * maintain some attachment classes of their own, we can probably repurpose this one...
         *
         * One issue: the images are already baked; so if we bake them here that's kinda wack and unneccessary.
         * Really I think only the draw commands need to actually bake.
         *
         * That means we should probably change this so that instead of baking the images, it just bakes the attachment
         * and hooks in the pre-existing images from the overall list of scene images. Then we just probably
         * copy in the shared pointers from the attachments
         *
         * @param sceneImageAttachmentList
         * @return
         */
        static std::vector<std::shared_ptr<gir::renderPass::ImageAttachment> >
        bakeListOfSceneImageAttachments(const std::vector<ImageAttachment> &sceneImageAttachmentList,
                                        const std::vector<std::shared_ptr<gir::ImageIR> > &allBakedImageGirs) {
            // first obtain only the images that are attached to this render pass through the given attachment list

            // build list of image attachment IR
            std::vector<std::shared_ptr<gir::renderPass::ImageAttachment> > imageAttachmentGirs = {};
            for (auto &imageAttachment: sceneImageAttachmentList) {
                imageAttachmentGirs.push_back(
                        createImageAttachment(
                                obtainAttachedImagesFromGlobalImagesList(sceneImageAttachmentList,
                                                                         allBakedImageGirs
                                ),
                                obtainAttachedImageGirIndices(
                                        obtainAttachedImagesFromGlobalImagesList(sceneImageAttachmentList,
                                                                                 allBakedImageGirs)
                                ),
                                imageAttachment)
                );
            }
            return imageAttachmentGirs;
        }

        /**
         *
         * @param attachedImageGirs
         * @param attachedImageGirIndices
         * @param imageAttachment
         * @return
         */
        static std::shared_ptr<gir::renderPass::ImageAttachment>
        createImageAttachment(const std::vector<std::shared_ptr<gir::ImageIR> > &attachedImageGirs,
                              const std::unordered_map<util::UniqueIdentifier, unsigned> &attachedImageGirIndices,
                              const ImageAttachment &imageAttachment) {
            return std::make_shared<gir::renderPass::ImageAttachment>(
                    gir::renderPass::ImageAttachment{
                            // obtain image directly from indices map
                            attachedImageGirs[attachedImageGirIndices.at(imageAttachment.image->getUid())],
                            convertImageAttachmentState(imageAttachment.imageState),
                            convertImageAttachmentMultisampleResolveOperation(imageAttachment.resolveOperation),
                            convertImageAttachmentResolveState(imageAttachment.resolveState),
                            getImageAttachmentLoadOperation(imageAttachment.loadOperation),
                            getImageAttachmentStoreOperation(imageAttachment.storeOperation),
                            getImageAttachmentClearValue(imageAttachment.clearValue)
                    }
            );
        }

        /**
         *
         * @param sceneImageAttachmentList
         * @param bakedSceneImageGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::ImageIR> > obtainAttachedImagesFromGlobalImagesList(
                const std::vector<ImageAttachment> &sceneImageAttachmentList,
                const std::vector<std::shared_ptr<gir::ImageIR> > &bakedSceneImageGirs) {
            std::vector<std::shared_ptr<gir::ImageIR> > attachedImageGirs;
            std::copy_if(bakedSceneImageGirs.begin(), bakedSceneImageGirs.end(), std::back_inserter(attachedImageGirs),
                         [&](const std::shared_ptr<gir::ImageIR> &imageGir) {
                             return std::any_of(sceneImageAttachmentList.begin(), sceneImageAttachmentList.end(),
                                                [&](const ImageAttachment &imageAttachment) {
                                                    return imageAttachment.image->getUid() == imageGir->getUid();
                                                });
                         });
            return attachedImageGirs;
        }

        /**
         *
         *
         * @param imageState
         * @return
         */
        static gir::renderPass::AttachmentState
        convertImageAttachmentState(const AttachmentState imageState) {
            static const std::unordered_map<scene::graph::renderPass::AttachmentState, gir::renderPass::AttachmentState>
                    conversionMap = {
                    {
                            scene::graph::renderPass::AttachmentState::UNDEFINED,
                            gir::renderPass::AttachmentState::UNDEFINED
                    },
                    {
                            scene::graph::renderPass::AttachmentState::PREINITIALIZED,
                            gir::renderPass::AttachmentState::PREINITIALIZED
                    },
                    {
                            scene::graph::renderPass::AttachmentState::COLOR_ATTACHMENT,
                            gir::renderPass::AttachmentState::COLOR_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::TRANSFER_SOURCE,
                            gir::renderPass::AttachmentState::TRANSFER_SOURCE
                    },
                    {
                            scene::graph::renderPass::AttachmentState::TRANSFER_DESTINATION,
                            gir::renderPass::AttachmentState::TRANSFER_DESTINATION
                    },
            };

            return conversionMap.at(imageState);
        }

        /**
         *
         * @param attachment
         * @return
         */
        static gir::renderPass::MultisampleResolveOperation
        convertImageAttachmentMultisampleResolveOperation(const MultisampleResolveOperation resolveOperation) {
            static const std::unordered_map<scene::graph::renderPass::MultisampleResolveOperation,
                    gir::renderPass::MultisampleResolveOperation>
                    conversionMap = {
                    {
                            scene::graph::renderPass::MultisampleResolveOperation::NO_OP,
                            gir::renderPass::MultisampleResolveOperation::NO_OP
                    },
                    {
                            scene::graph::renderPass::MultisampleResolveOperation::SET_TO_FIRST_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::SET_TO_FIRST_SAMPLE
                    },
                    {
                            scene::graph::renderPass::MultisampleResolveOperation::AVERAGE_OF_ALL_SAMPLES,
                            gir::renderPass::MultisampleResolveOperation::AVERAGE_OF_ALL_SAMPLES
                    },
                    {
                            scene::graph::renderPass::MultisampleResolveOperation::MINIMUM_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::MINIMUM_SAMPLE
                    },
                    {
                            scene::graph::renderPass::MultisampleResolveOperation::MAXIMUM_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::MAXIMUM_SAMPLE
                    }
            };

            return conversionMap.at(resolveOperation);
        }

        /**
         *
         * @param attachment
         * @return
         */
        static gir::renderPass::AttachmentState convertImageAttachmentResolveState(const AttachmentState resolveState) {
            static const std::unordered_map<scene::graph::renderPass::AttachmentState, gir::renderPass::AttachmentState>
                    conversionMap = {
                    {
                            scene::graph::renderPass::AttachmentState::UNDEFINED,
                            gir::renderPass::AttachmentState::UNDEFINED
                    },
                    {
                            scene::graph::renderPass::AttachmentState::PREINITIALIZED,
                            gir::renderPass::AttachmentState::PREINITIALIZED
                    },
                    {
                            scene::graph::renderPass::AttachmentState::COLOR_ATTACHMENT,
                            gir::renderPass::AttachmentState::COLOR_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::DEPTH_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_ATTACHMENT
                    },
                    {
                            scene::graph::renderPass::AttachmentState::TRANSFER_SOURCE,
                            gir::renderPass::AttachmentState::TRANSFER_SOURCE
                    },
                    {
                            scene::graph::renderPass::AttachmentState::TRANSFER_DESTINATION,
                            gir::renderPass::AttachmentState::TRANSFER_DESTINATION
                    },
            };

            return conversionMap.at(resolveState);
        }

        /**
         *
         * @param attachment
         * @return
         */
        static gir::renderPass::AttachmentLoadOperation
        getImageAttachmentLoadOperation(const AttachmentLoadOperation attachmentLoadOperation) {
            static const std::unordered_map<scene::graph::renderPass::AttachmentLoadOperation,
                    gir::renderPass::AttachmentLoadOperation> conversionMap = {
                    {
                            scene::graph::renderPass::AttachmentLoadOperation::DONT_CARE,
                            gir::renderPass::AttachmentLoadOperation::DONT_CARE
                    },
                    {
                            scene::graph::renderPass::AttachmentLoadOperation::LOAD,
                            gir::renderPass::AttachmentLoadOperation::LOAD
                    },
                    {
                            scene::graph::renderPass::AttachmentLoadOperation::CLEAR,
                            gir::renderPass::AttachmentLoadOperation::CLEAR
                    }
            };

            return conversionMap.at(attachmentLoadOperation);
        }

        /**
         *
         * @param attachment
         * @return
         */
        static gir::renderPass::AttachmentStoreOperation
        getImageAttachmentStoreOperation(const AttachmentStoreOperation attachmentStoreOperation) {
            static const std::unordered_map<scene::graph::renderPass::AttachmentStoreOperation,
                    gir::renderPass::AttachmentStoreOperation> conversionMap = {
                    {
                            scene::graph::renderPass::AttachmentStoreOperation::DONT_CARE,
                            gir::renderPass::AttachmentStoreOperation::DONT_CARE
                    },
                    {
                            scene::graph::renderPass::AttachmentStoreOperation::STORE,
                            gir::renderPass::AttachmentStoreOperation::STORE
                    },
            };

            return conversionMap.at(attachmentStoreOperation);
        }

        /**
         *
         * @param attachmentClearValue
         * @return
         */
        static gir::renderPass::ClearValue getImageAttachmentClearValue(const ClearValue &attachmentClearValue) {
            return gir::renderPass::ClearValue{
                    attachmentClearValue.floatClearValue,
                    attachmentClearValue.intClearValue,
                    attachmentClearValue.uintClearValue
            };
        }

        /**
         *
         * @return
         */
        static std::unordered_map<UniqueIdentifier, unsigned> obtainAttachedImageGirIndices(
                const std::vector<std::shared_ptr<gir::ImageIR> > &attachedImageGirs
        ) {
            std::unordered_map<UniqueIdentifier, unsigned> attachedImageGirIndices = {};
            int index = 0;
            for (const auto &attachedImageGir: attachedImageGirs) {
                attachedImageGirIndices[attachedImageGir->getUid()] = index;
                index++;
            }
            return attachedImageGirIndices;
        }

        /**
         * Obtains a list of all the image IR objects from the global list of all resource IR objects.
         * TODO - refactor this to be templated (if possible) so we don't need to duplicate this for different IR destination classes
         * @param sceneResourceGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::ImageIR> > obtainImageGirsFromListOfAllResourceGirs(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneResourceGirs) {
            // copy only the images into the imageGirs list
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > imageGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(imageGirs),
                         [&](const std::shared_ptr<gir::GraphicsIntermediateRepresentation> &sceneResourceGir) {
                             return sceneResourceGir->getSubtype() == gir::GIRSubtype::IMAGE;
                         });
            // cast the imageGirs to their actual pointer type (ImageIR)
            std::vector<std::shared_ptr<gir::ImageIR> > convertedImageGirs = {};
            std::transform(imageGirs.begin(), imageGirs.end(), std::back_inserter(convertedImageGirs),
                           [&](std::shared_ptr<gir::GraphicsIntermediateRepresentation> &gir) {
                               // cast the shared pointer to ImageIR
                               return std::dynamic_pointer_cast<gir::ImageIR>(gir);
                           });

            return convertedImageGirs;
        }

        // TEXTURE BAKE HELPER FUNCTIONS

        /**
         * Given a list of images, find the one specified in the given attachment
         * @param imageGirs
         * @param attachment
         * @return
         */
        static std::shared_ptr<gir::ImageIR>
        findTextureImageIRFromList(const std::vector<std::shared_ptr<gir::ImageIR> > &imageGirs,
                                   const TextureAttachment &attachment) {
            const auto &findAttachedImageIterator = std::find_if(imageGirs.begin(), imageGirs.end(),
                                                                 [&](const std::shared_ptr<gir::ImageIR> &imageGir) {
                                                                     return imageGir->getUid() ==
                                                                            attachment.textureResource->getUid();
                                                                 });
            if (findAttachedImageIterator == imageGirs.end()) {
                return nullptr;
            }

            return *findAttachedImageIterator;
        }

        /**
         *
         * @param attachedTextureImageGirs
         * @param textureAttachment
         * @param textureResource
         * @param sampler
         * @return
         */
        static std::shared_ptr<gir::renderPass::TextureAttachment>
        createTextureAttachment(std::vector<std::shared_ptr<gir::ImageIR> > &attachedTextureImageGirs,
                                const TextureAttachment &textureAttachment,
                                const std::shared_ptr<Texture> &textureResource,
                                const SamplerSettings &sampler) {
            return std::make_shared<gir::renderPass::TextureAttachment>(
                    gir::renderPass::TextureAttachment{
                            findTextureImageIRFromList(attachedTextureImageGirs, textureAttachment),
                            convertShaderStages(textureAttachment.shaderStages),
                            textureResource->getMipLevelOfDetailBias(),
                            textureResource->getMaxAnisotropy(),
                            convertMagnificationFilterType(
                                    textureResource->getMagnificationFilterType()),
                            convertMinificationFilterType(
                                    textureResource->getMinificationFilterType()),
                            convertSamplerMipmapMode(
                                    textureResource->getSamplerMipmapMode()),
                            convertOutOfBoundsTexelCoordinateAddressMode(
                                    textureResource->getAddressMode()),
                            convertPercentageCloserFilteringCompareOperation(
                                    textureResource->getPcfCompareOperation()),
                            sampler.isSamplerAnisotropyEnabled,
                            sampler.minimumLod,
                            sampler.maximumLod,
                            sampler.isSamplerUsingUnnormalizedCoordinates,
                            sampler.isSamplerPCFEnabled
                    });
        }

        /**
         *
         * @param textureAttachments
         * @param sceneImageGirs
         * @param attachedTextureImageGirs
         */
        static void
        copyAttachedTextureImagesFromGlobalResourceGirList(const std::vector<TextureAttachment> &textureAttachments,
                                                           const std::vector<std::shared_ptr<gir::ImageIR> > &
                                                           sceneImageGirs,
                                                           std::vector<std::shared_ptr<gir::ImageIR> > &
                                                           attachedTextureImageGirs) {
            std::copy_if(sceneImageGirs.begin(), sceneImageGirs.end(), std::back_inserter(attachedTextureImageGirs),
                         [&](const std::shared_ptr<gir::ImageIR> &image) {
                             return std::any_of(textureAttachments.begin(), textureAttachments.end(),
                                                [&](const TextureAttachment &textureAttachment) {
                                                    return textureAttachment.textureResource->getSampledImage()->
                                                            getUid()
                                                           == image->getUid();
                                                });
                         });
        }

        /**
         *
         * @param sceneShaderStages
         * @return
         */
        static std::vector<gir::renderPass::TextureAttachment::ShaderStage>
        convertShaderStages(
                const std::vector<scene::graph::renderPass::TextureAttachment::ShaderStage> &sceneShaderStages) {
            // TODO - replace this 2-stage "implementation" with an actual conversion map (or whichever other method)
            std::vector<gir::renderPass::TextureAttachment::ShaderStage> shaderStages = {};
            for (const auto &sceneShaderStage: sceneShaderStages) {
                gir::renderPass::TextureAttachment::ShaderStage convertedShaderStage
                        = (sceneShaderStage ==
                           scene::graph::renderPass::TextureAttachment::ShaderStage::VERTEX)
                          ? gir::renderPass::TextureAttachment::ShaderStage::VERTEX
                          : gir::renderPass::TextureAttachment::ShaderStage::FRAGMENT;
                shaderStages.push_back(convertedShaderStage);
            }

            return shaderStages;
        }

        /**
         *
         * @param magnificationFilterType
         * @return
         */
        static gir::renderPass::TextureAttachment::MagnificationFilterType
        convertMagnificationFilterType(SamplerSettings::MagnificationFilterType magnificationFilterType) {
            static const std::unordered_map<
                    SamplerSettings::MagnificationFilterType,
                    gir::renderPass::TextureAttachment::MagnificationFilterType
            > MAGNIFICATION_FILTER_CONVERSION_MAP = {
                    {
                            SamplerSettings::MagnificationFilterType::NEAREST_TEXEL,
                            gir::renderPass::TextureAttachment::MagnificationFilterType::NEAREST_TEXEL
                    },
                    {
                            SamplerSettings::MagnificationFilterType::LINEAR_BLEND,
                            gir::renderPass::TextureAttachment::MagnificationFilterType::LINEAR_BLEND
                    },
            };

            return MAGNIFICATION_FILTER_CONVERSION_MAP.at(magnificationFilterType);
        }

        /**
         *
         * @param minificationFilterType
         * @return
         */
        static gir::renderPass::TextureAttachment::MinificationFilterType
        convertMinificationFilterType(SamplerSettings::MinificationFilterType minificationFilterType) {
            static const std::unordered_map<
                    SamplerSettings::MinificationFilterType,
                    gir::renderPass::TextureAttachment::MinificationFilterType
            > MINIFICATION_FILTER_CONVERSION_MAP = {
                    {
                            SamplerSettings::MinificationFilterType::NEAREST_TEXEL,
                            gir::renderPass::TextureAttachment::MinificationFilterType::NEAREST_TEXEL
                    },
                    {
                            SamplerSettings::MinificationFilterType::LINEAR_BLEND,
                            gir::renderPass::TextureAttachment::MinificationFilterType::LINEAR_BLEND
                    },
            };

            return MINIFICATION_FILTER_CONVERSION_MAP.at(minificationFilterType);
        }

        /**
         *
         * @param operation
         * @return
         */
        static gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation
        convertPercentageCloserFilteringCompareOperation(
                SamplerSettings::PercentageCloserFilteringCompareOperation operation) {
            static const std::unordered_map<
                    SamplerSettings::PercentageCloserFilteringCompareOperation,
                    gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation
            > PCF_COMPARE_OPERATION_CONVERSION_MAP = {
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::ALWAYS,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::ALWAYS
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::NEVER,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::NEVER
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::GREATER_THAN_EQUALS,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::GREATER_THAN_EQUALS
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::GREATER_THAN,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::GREATER_THAN
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::EQUALS,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::EQUALS
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::NOT_EQUALS,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::NOT_EQUALS
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::LESS_THAN,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::LESS_THAN
                    },
                    {
                            SamplerSettings::PercentageCloserFilteringCompareOperation::LESS_THAN_EQUALS,
                            gir::renderPass::TextureAttachment::PercentageCloserFilteringCompareOperation::LESS_THAN_EQUALS
                    },
            };

            return PCF_COMPARE_OPERATION_CONVERSION_MAP.at(operation);
        }

        /**
         *
         * @param mode
         * @return
         */
        static gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode
        convertOutOfBoundsTexelCoordinateAddressMode(SamplerSettings::OutOfBoundsTexelCoordinateAddressMode mode) {
            static const std::unordered_map<
                    SamplerSettings::OutOfBoundsTexelCoordinateAddressMode,
                    gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode
            > OOB_TEXEL_COORDS_ADDRESS_MODE_CONVERSION_MAP = {
                    {
                            SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::REPEAT,
                            gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode::REPEAT
                    },
                    {
                            SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT,
                            gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT
                    },
                    {
                            SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_EDGE,
                            gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_EDGE
                    },
                    {
                            SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_BORDER,
                            gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_BORDER
                    },
                    {
                            SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::MIRROR_CLAMP_TO_EDGE,
                            gir::renderPass::TextureAttachment::OutOfBoundsTexelCoordinateAddressMode::MIRROR_CLAMP_TO_EDGE
                    },
            };

            return OOB_TEXEL_COORDS_ADDRESS_MODE_CONVERSION_MAP.at(mode);
        }

        /**
         *
         * @param mode
         * @return
         */
        static gir::renderPass::TextureAttachment::SamplerMipmapMode
        convertSamplerMipmapMode(SamplerSettings::SamplerMipmapMode mode) {
            static const std::unordered_map<
                    SamplerSettings::SamplerMipmapMode,
                    gir::renderPass::TextureAttachment::SamplerMipmapMode
            > SAMPLER_MIPMAP_MODE_CONVERSION_MAP = {
                    {
                            SamplerSettings::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER,
                            gir::renderPass::TextureAttachment::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER
                    },
                    {
                            SamplerSettings::SamplerMipmapMode::LINEAR_BLEND,
                            gir::renderPass::TextureAttachment::SamplerMipmapMode::LINEAR_BLEND
                    },
            };

            return SAMPLER_MIPMAP_MODE_CONVERSION_MAP.at(mode);
        }

        // SHADER CONSTANT BAKE HELPER FUNCTIONS

        /**
         * TODO - replace all these different functions with a single template function
         * @param sceneResourceGirs
         * @return
         */
        static std::vector<std::shared_ptr<gir::ShaderConstantIR> > obtainShaderConstantGirsFromListOfAllResourceGirs(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneResourceGirs) {
            // copy only the shader constants into the shaderConstantGirs list
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > shaderConstantGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(shaderConstantGirs),
                         [&](const std::shared_ptr<gir::GraphicsIntermediateRepresentation> &sceneResourceGir) {
                             return sceneResourceGir->getSubtype() == gir::GIRSubtype::SHADER_CONSTANT;
                         });
            // cast the shaderConstantGirs to their actual pointer type (ShaderConstantIR)
            std::vector<std::shared_ptr<gir::ShaderConstantIR> > convertedShaderConstantGirs = {};
            std::transform(shaderConstantGirs.begin(), shaderConstantGirs.end(),
                           std::back_inserter(convertedShaderConstantGirs),
                           [&](std::shared_ptr<gir::GraphicsIntermediateRepresentation> &gir) {
                               return std::dynamic_pointer_cast<gir::ShaderConstantIR>(gir);
                           });
            return convertedShaderConstantGirs;
        }

        // BUFFER BAKE HELPER FUNCTIONS

        static std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> >
        bakeListOfBufferAttachments(const std::vector<BufferAttachment> &bufferAttachments,
                                    const std::vector<std::shared_ptr<gir::BufferIR> > &sceneBufferGirs) {
            std::vector<std::shared_ptr<gir::renderPass::BufferAttachment> > bufferAttachmentGirs = {};

            for (const auto &attachment: bufferAttachments) {
                const auto &attachedBufferGirItr
                        = std::find_if(sceneBufferGirs.begin(), sceneBufferGirs.end(),
                                       [&](const std::shared_ptr<gir::BufferIR> &buffer) {
                                           return buffer->getUid() ==
                                                  attachment.buffer->getUid();
                                       });

                if (attachedBufferGirItr == sceneBufferGirs.end()) {
                    // TODO - better logging
                    throw std::runtime_error(
                            "Error in scene::RenderPass::bakeListOfBufferAttachments() - attached buffer not found!");
                }

                bufferAttachmentGirs.push_back(std::make_shared<gir::renderPass::BufferAttachment>(
                        gir::renderPass::BufferAttachment{
                                *(attachedBufferGirItr)
                        }
                ));
            }

            return bufferAttachmentGirs;
        }

        // SHADER ATTACHMENT BAKE HELPER FUNCTIONS

        [[nodiscard]] static std::vector<std::shared_ptr<gir::ShaderModuleIR> > obtainShaderModuleGirsList(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &sceneShaderModuleGirs) {
            std::vector<std::shared_ptr<gir::ShaderModuleIR> > sceneShaderModules = {};
            std::transform(sceneShaderModuleGirs.begin(), sceneShaderModuleGirs.end(),
                           std::back_inserter(sceneShaderModules),
                           [&](const std::shared_ptr<gir::GraphicsIntermediateRepresentation> &sceneShaderModuleGir) {
                               return std::dynamic_pointer_cast<gir::ShaderModuleIR>(sceneShaderModuleGir);
                           });
            return sceneShaderModules;
        }

        static std::shared_ptr<ShaderGirAttachment>
        bakeShaderAttachment(const ShaderAttachment &shaderAttachment,
                             const std::shared_ptr<gir::ShaderModuleIR> &attachedShaderModuleGir) {
            // TODO (if necessary) - add any other information that we want to carry as part of the shader attachment
            return std::make_shared<ShaderGirAttachment>(attachedShaderModuleGir);
        }

        // GRAPHICS PIPELINE CONFIGURATION BAKE HELPER FUNCTIONS

        /**
         *
         * @param configuration
         * @param logicOperationConversionMap
         * @param blendFactorConversionMap
         * @param blendOperationConversionMap
         * @param colorComponentConversionMap
         * @return
         */
        static ColorBlendStateIR createColorBlendStateGir(
                const ColorBlendConfiguration &configuration,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::LogicOperation,
                        gir::pipeline::ColorBlendStateIR::LogicOperation> &logicOperationConversionMap,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor,
                        gir::pipeline::ColorBlendStateIR::BlendFactor> &blendFactorConversionMap,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation,
                        gir::pipeline::ColorBlendStateIR::BlendOperation> &blendOperationConversionMap,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent,
                        gir::pipeline::ColorBlendStateIR::ColorComponent> &colorComponentConversionMap) {
            std::vector<ColorBlendStateIR::ColorAttachmentBlendState> colorAttachmentBlendStateGirs
                    = bakePipelineColorAttachmentBlendStateGirs(
                            configuration,
                            blendFactorConversionMap,
                            blendOperationConversionMap,
                            colorComponentConversionMap);

            return {
                    configuration.enableColorBlend,
                    configuration.enableColorBlendLogicOperation,
                    logicOperationConversionMap.at(configuration.logicOp),
                    configuration.blendConstants,
                    colorAttachmentBlendStateGirs
            };
        }

        /**
         *
         * @param configuration
         * @param blendFactorConversionMap
         * @param blendOperationConversionMap
         * @param colorComponentConversionMap
         * @return
         */
        static std::vector<ColorBlendStateIR::ColorAttachmentBlendState>
        bakePipelineColorAttachmentBlendStateGirs(
                const ColorBlendConfiguration &configuration,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendFactor,
                        gir::pipeline::ColorBlendStateIR::BlendFactor> &blendFactorConversionMap,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::BlendOperation,
                        gir::pipeline::ColorBlendStateIR::BlendOperation> &blendOperationConversionMap,
                const std::unordered_map<scene::graph::renderPass::pipeline::ColorBlendConfiguration::ColorComponent,
                        gir::pipeline::ColorBlendStateIR::ColorComponent> &colorComponentConversionMap) {
            std::vector<ColorBlendStateIR::ColorAttachmentBlendState> colorAttachmentBlendStateGirs = {};

            for (const auto &sceneColorAttachmentBlendState: configuration.colorAttachmentBlendStates) {
                std::unordered_set<ColorBlendStateIR::ColorComponent> enabledBlendComponents = {};
                for (const auto &enabledColorComponent: sceneColorAttachmentBlendState.enabledBlendComponents) {
                    enabledBlendComponents.insert(colorComponentConversionMap.at(enabledColorComponent));
                }

                colorAttachmentBlendStateGirs.push_back(ColorBlendStateIR::ColorAttachmentBlendState{
                        sceneColorAttachmentBlendState.enableColorBlendingForThisAttachment,
                        blendFactorConversionMap.at(sceneColorAttachmentBlendState.sourceColorBlendFactor),
                        blendFactorConversionMap.at(sceneColorAttachmentBlendState.destinationColorBlendFactor),
                        blendOperationConversionMap.at(sceneColorAttachmentBlendState.colorBlendOp),
                        blendFactorConversionMap.at(sceneColorAttachmentBlendState.sourceAlphaBlendFactor),
                        blendFactorConversionMap.at(sceneColorAttachmentBlendState.destinationAlphaBlendFactor),
                        blendOperationConversionMap.at(sceneColorAttachmentBlendState.alphaBlendOp),
                        enabledBlendComponents
                });
            }
            return colorAttachmentBlendStateGirs;
        }

        /**
         *
         * @param configuration
         * @param compareOpConversionMap
         * @param stencilOpConversionMap
         * @return
         */
        static DepthStencilStateIR createDepthStencilStateGir(
                const DepthStencilConfiguration &configuration,
                std::unordered_map<DepthStencilConfiguration::CompareOperation, DepthStencilStateIR::CompareOperation>
                compareOpConversionMap,
                std::unordered_map<DepthStencilConfiguration::StencilOperation, DepthStencilStateIR::StencilOperation>
                stencilOpConversionMap) {
            return {
                    configuration.enableDepthTesting,
                    configuration.enableDepthWrites,
                    compareOpConversionMap.at(configuration.depthTestingOperation),
                    configuration.enableDepthBoundsTest,
                    configuration.depthBoundMinimumValue,
                    configuration.depthBoundMinimumValue,
                    configuration.enableStencilTest,
                    {
                            stencilOpConversionMap.at(configuration.backFacingStencilTestState.depthFailOp),
                            stencilOpConversionMap.at(configuration.backFacingStencilTestState.passOp),
                            stencilOpConversionMap.at(configuration.backFacingStencilTestState.failOp),
                            compareOpConversionMap.at(configuration.backFacingStencilTestState.compareOp),
                            configuration.backFacingStencilTestState.compareMask,
                            configuration.backFacingStencilTestState.writeMask,
                            configuration.backFacingStencilTestState.reference
                    },
                    {
                            stencilOpConversionMap.at(configuration.frontFacingStencilTestState.depthFailOp),
                            stencilOpConversionMap.at(configuration.frontFacingStencilTestState.passOp),
                            stencilOpConversionMap.at(configuration.frontFacingStencilTestState.failOp),
                            compareOpConversionMap.at(configuration.frontFacingStencilTestState.compareOp),
                            configuration.frontFacingStencilTestState.compareMask,
                            configuration.frontFacingStencilTestState.writeMask,
                            configuration.frontFacingStencilTestState.reference
                    }
            };
        }

        /**
         *
         * @param configuration
         * @param dynamicStateConversionMap
         * @return
         */
        static DynamicStateIR createDynamicStateGir(
                const DynamicStateConfiguration &configuration,
                const std::unordered_map<DynamicStateConfiguration::DynamicStateSource, DynamicStateIR::DynamicStateSource>
                &dynamicStateConversionMap) {
            std::unordered_set<DynamicStateIR::DynamicStateSource> dynamicStateSources = {};

            for (const auto &sceneDynamicStateSource: configuration.dynamicStateSources) {
                dynamicStateSources.insert(dynamicStateConversionMap.at(sceneDynamicStateSource));
            }

            return {
                    configuration.dynamicStateEnabled,
                    dynamicStateSources
            };
        }

        /**
         *
         * @param configuration
         * @param numberOfSamplesConversionMap
         * @return
         */
        static MultisampleStateIR createMultisampleStateGir(
                const MultisampleConfiguration &configuration,
                const std::unordered_map<MultisampleConfiguration::NumberOfSamples, MultisampleStateIR::NumberOfSamples> &
                numberOfSamplesConversionMap) {
            return {
                    configuration.enableMultisampling,
                    numberOfSamplesConversionMap.at(configuration.minimumNumberOfRasterizationSamples),
                    configuration.enableSampleShading,
                    configuration.fractionOfSamplesToShade,
                    configuration.sampleMask,
                    configuration.enableAlphaToCoverage,
                    configuration.enableAlphaToOne
            };
        }

        /**
         *
         * @param configuration
         * @param primitiveTopologyConversionMap
         * @return
         */
        static PrimitiveAssemblyIR createPrimitiveAssemblyGir(
                const PrimitiveAssemblyConfiguration &configuration,
                const std::unordered_map<PrimitiveAssemblyConfiguration::PrimitiveTopology,
                        PrimitiveAssemblyIR::PrimitiveTopology> &primitiveTopologyConversionMap) {
            std::unordered_set<PrimitiveAssemblyIR::PrimitiveTopology> enabledPrimitiveTopologies = {};

            for (const auto &enabledPrimitiveTopology: configuration.enabledPrimitiveTopologies) {
                enabledPrimitiveTopologies.insert(primitiveTopologyConversionMap.at(enabledPrimitiveTopology));
            }

            return {
                    configuration.enablePrimitiveRestartForIndexedDraws,
                    enabledPrimitiveTopologies
            };
        }

        /**
         *
         * @param configuration
         * @param rasterizationModeConversionMap
         * @param polygonCullingModeConversionMap
         * @param frontFaceOrientationModeConversionMap
         * @return
         */
        static RasterizationStateIR createRasterizationStateGir(const RasterizationConfiguration &configuration,
                                                                const std::unordered_map<
                                                                        RasterizationConfiguration::PolygonRasterizationMode, RasterizationStateIR::PolygonRasterizationMode> &
                                                                rasterizationModeConversionMap,
                                                                const std::unordered_map<
                                                                        RasterizationConfiguration::PolygonCullingMode,
                                                                        RasterizationStateIR::PolygonCullingMode> &
                                                                polygonCullingModeConversionMap,
                                                                const std::unordered_map<
                                                                        RasterizationConfiguration::PolygonFrontFaceOrientationMode, RasterizationStateIR::PolygonFrontFaceOrientationMode>
                                                                &frontFaceOrientationModeConversionMap) {
            return {
                    configuration.enableDepthClamping,
                    configuration.enableExtendedDepthClipState,
                    {
                            // TODO - add any parameters that get added to this extended depth state struct
                    },
                    configuration.discardAllPrimitivesBeforeRasterization,
                    rasterizationModeConversionMap.at(configuration.rasterizationMode),
                    polygonCullingModeConversionMap.at(configuration.cullingMode),
                    frontFaceOrientationModeConversionMap.at(configuration.polygonFrontFaceOrientation),
                    configuration.lineWidth,
                    configuration.enableDepthBias,
                    configuration.depthBiasAdditiveConstantValue,
                    configuration.depthBiasClamp,
                    configuration.depthBiasSlopeFactor
            };
        }
    };
}
