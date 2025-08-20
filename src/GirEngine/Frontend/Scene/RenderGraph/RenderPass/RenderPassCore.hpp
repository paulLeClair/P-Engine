//
// Created by paull on 2024-10-08.
//

#pragma once

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "ImageAttachment/ImageAttachment.hpp"
#include "DrawAttachment/DrawAttachment.hpp"
#include "ShaderConstantAttachment/ShaderConstantAttachment.hpp"
#include "TextureAttachment/TextureAttachment.hpp"
#include "ShaderAttachment/ShaderAttachment.hpp"
#include "GraphicsPipelineConfiguration/GraphicsPipelineConfiguration.hpp"


#include "../../../GraphicsIR/RenderPassIR/StaticRenderPassIR/StaticRenderPassIR.hpp"
#include "../../Scene.hpp"

#include <string>
#include <utility>
#include <vector>

namespace pEngine::girEngine::scene::graph::renderPass {
    using namespace pipeline;

    struct RenderPassCore {
        std::string name;
        UniqueIdentifier uid;

        GraphicsPipelineConfiguration graphicsPipelineConfiguration;

        /**
          * This defines the vertex & index buffer binding interfaces that exist for your render pass
         */
        std::vector<geometry::GeometryBinding> geometryBindings = {};
        std::vector<DrawAttachment> drawBindings = {};

        std::vector<ImageAttachment> colorAttachments = {};

        std::vector<Buffer> uniformBuffers = {};
        // TODO -> support other buffer types following similar pattern

        // shader constant attachments (largely TODO)
        std::vector<ShaderConstantAttachment> shaderConstants = {};

        // texture attachments
        std::vector<TextureAttachment> textureAttachments = {};

        // for now, just hardcoding the classic pipeline shader stages (will be extended in the future)
        ShaderAttachment vertexShaderAttachment = {};
        ShaderAttachment fragmentShaderAttachment = {};

        // also for now, depth attachment functionality is quite rigid (but will be changed during render graph refactor)
        ImageAttachment depthAttachment;
        bool depthTestEnabled;
        bool depthWriteEnabled;

        void bindModelsForDrawing(unsigned targetGeometryBindingIndex,
                                  const std::vector<Model> &boundModels) {
            drawBindings.push_back({targetGeometryBindingIndex, boundModels});
        }

        bool setFragmentShaderModule(const ShaderAttachment &shaderModule) {
            fragmentShaderAttachment = shaderModule;
            return true;
        }

        static std::vector<gir::renderPass::TextureAttachment>
        bakeTextureAttachments(const std::vector<TextureAttachment> &textureAttachments) {
            std::vector<gir::ImageIR> attachedTextureImageGirs = {};
            // TODO -> implement this when we add texture support

            if (attachedTextureImageGirs.empty()) {
                return {};
            }

            // build texture attachment girs - TODO/future update
            std::vector<gir::renderPass::TextureAttachment> textureAttachmentGirs = {};
            for (const auto &textureAttachment: textureAttachments) {
            }
            return textureAttachmentGirs;
        }

        static std::vector<gir::ShaderConstantIR>
        bakeShaderConstantAttachments(const std::vector<ShaderConstantAttachment> &shaderConstantAttachments,
                                      const std::vector<gir::ShaderConstantIR> &shaderConstantGirs) {
            std::vector<gir::ShaderConstantIR> shaderConstantAttachmentGirs = {};

            for (auto &renderPassShaderConstantAttachment: shaderConstantAttachmentGirs) {
                // TODO
            }

            return shaderConstantAttachmentGirs;
        }

        static std::vector<BufferAttachment>
        bakeUniformBufferAttachments(const std::vector<Buffer> &uniformBufferAttachments) {
            std::vector<BufferAttachment> attachments = {};
            for (auto &buffer: uniformBufferAttachments) {
                attachments.push_back(BufferAttachment{
                    gir::BufferIR{
                        buffer.name,
                        buffer.uid,
                        gir::GIRSubtype::BUFFER,
                        gir::BufferIR::BufferUsage::UNIFORM_BUFFER,
                        buffer.getBindingIndex(),
                        buffer.getRawDataContainer().getRawDataByteArray(),
                        static_cast<uint32_t>(buffer.getRawDataContainer().getRawDataSizeInBytes())
                    }
                });
            }

            return attachments;
        }


        static gir::SpirVShaderModuleIR
        findVertexShaderModule(
            const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) {
            for (auto &shaderModuleGir: sceneShaderModuleGirs) {
                if (shaderModuleGir.getUsage() == gir::SpirVShaderModuleIR::ShaderUsage::VERTEX_SHADER) {
                    return shaderModuleGir;
                }
            }

            // TODO -> log in this case
            return {};
        }

        static gir::SpirVShaderModuleIR
        findFragmentShaderModule(
            const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) {
            for (auto &shaderModuleGir: sceneShaderModuleGirs) {
                if (shaderModuleGir.getUsage() == gir::SpirVShaderModuleIR::ShaderUsage::FRAGMENT_SHADER) {
                    return shaderModuleGir;
                }
            }

            // TODO -> log in this case
            return {};
        }

        static ColorBlendStateIR bakeColorBlendState(const ColorBlendConfiguration &configuration) {
            // for now, some static conversion maps for any enums will probably do... eventually we'll refactor this

            // define logic operation conversion map
            static const std::unordered_map<
                ColorBlendConfiguration::LogicOperation,
                ColorBlendStateIR::LogicOperation
            > LOGIC_OPERATION_CONVERSION_MAP
                    = {
                        {
                            ColorBlendConfiguration::LogicOperation::NO_OP,
                            ColorBlendStateIR::LogicOperation::NO_OP
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::CLEAR,
                            ColorBlendStateIR::LogicOperation::CLEAR
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::AND,
                            ColorBlendStateIR::LogicOperation::AND
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::AND_REVERSE,
                            ColorBlendStateIR::LogicOperation::AND_REVERSE
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::COPY,
                            ColorBlendStateIR::LogicOperation::COPY
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::AND_INVERTED,
                            ColorBlendStateIR::LogicOperation::AND_INVERTED
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::XOR,
                            ColorBlendStateIR::LogicOperation::XOR
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::OR,
                            ColorBlendStateIR::LogicOperation::OR
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::NOR,
                            ColorBlendStateIR::LogicOperation::NOR
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::EQUIVALENT,
                            ColorBlendStateIR::LogicOperation::EQUIVALENT
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::INVERT,
                            ColorBlendStateIR::LogicOperation::INVERT
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::OR_REVERSE,
                            ColorBlendStateIR::LogicOperation::OR_REVERSE
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::COPY_INVERTED,
                            ColorBlendStateIR::LogicOperation::COPY_INVERTED
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::OR_INVERTED,
                            ColorBlendStateIR::LogicOperation::OR_INVERTED
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::NAND,
                            ColorBlendStateIR::LogicOperation::NAND
                        },
                        {
                            ColorBlendConfiguration::LogicOperation::SET,
                            ColorBlendStateIR::LogicOperation::SET
                        },
                    };
            // define blend factor conversion map
            static const std::unordered_map<
                ColorBlendConfiguration::BlendFactor,
                ColorBlendStateIR::BlendFactor
            > BLEND_FACTOR_CONVERSION_MAP
                    = {
                        {
                            ColorBlendConfiguration::BlendFactor::ZERO,
                            ColorBlendStateIR::BlendFactor::ZERO
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE,
                            ColorBlendStateIR::BlendFactor::ONE
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::SRC_COLOR,
                            ColorBlendStateIR::BlendFactor::SRC_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC_COLOR,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::DST_COLOR,
                            ColorBlendStateIR::BlendFactor::DST_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_DST_COLOR,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_DST_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::SRC_ALPHA,
                            ColorBlendStateIR::BlendFactor::SRC_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC_ALPHA,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::DST_ALPHA,
                            ColorBlendStateIR::BlendFactor::DST_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_DST_ALPHA,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_DST_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::CONSTANT_COLOR,
                            ColorBlendStateIR::BlendFactor::CONSTANT_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_CONSTANT_COLOR,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_CONSTANT_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::CONSTANT_ALPHA,
                            ColorBlendStateIR::BlendFactor::CONSTANT_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_CONSTANT_ALPHA,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_CONSTANT_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::SRC_ALPHA_SATURATE,
                            ColorBlendStateIR::BlendFactor::SRC_ALPHA_SATURATE
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::SRC1_COLOR,
                            ColorBlendStateIR::BlendFactor::SRC1_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC1_COLOR,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC1_COLOR
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::SRC1_ALPHA,
                            ColorBlendStateIR::BlendFactor::SRC1_ALPHA
                        },
                        {
                            ColorBlendConfiguration::BlendFactor::ONE_MINUS_SRC1_ALPHA,
                            ColorBlendStateIR::BlendFactor::ONE_MINUS_SRC1_ALPHA
                        },
                    };
            // define blend operation conversion map
            static const std::unordered_map<
                ColorBlendConfiguration::BlendOperation,
                ColorBlendStateIR::BlendOperation
            > BLEND_OPERATION_CONVERSION_MAP
                    = {
                        {
                            ColorBlendConfiguration::BlendOperation::NO_OP,
                            ColorBlendStateIR::BlendOperation::NO_OP
                        },
                        {
                            ColorBlendConfiguration::BlendOperation::ADD,
                            ColorBlendStateIR::BlendOperation::ADD
                        },
                        {
                            ColorBlendConfiguration::BlendOperation::SUBTRACT,
                            ColorBlendStateIR::BlendOperation::SUBTRACT
                        },
                        {
                            ColorBlendConfiguration::BlendOperation::REVERSE_SUBTRACT,
                            ColorBlendStateIR::BlendOperation::REVERSE_SUBTRACT
                        },
                        {
                            ColorBlendConfiguration::BlendOperation::MIN,
                            ColorBlendStateIR::BlendOperation::MIN
                        },
                        {
                            ColorBlendConfiguration::BlendOperation::MAX,
                            ColorBlendStateIR::BlendOperation::MAX
                        },
                    };
            // define color component conversion map
            static const std::unordered_map<
                ColorBlendConfiguration::ColorComponent,
                ColorBlendStateIR::ColorComponent
            > COLOR_COMPONENT_CONVERSION_MAP
                    = {
                        {
                            ColorBlendConfiguration::ColorComponent::RED,
                            ColorBlendStateIR::ColorComponent::RED
                        },
                        {
                            ColorBlendConfiguration::ColorComponent::GREEN,
                            ColorBlendStateIR::ColorComponent::GREEN
                        },
                        {
                            ColorBlendConfiguration::ColorComponent::BLUE,
                            ColorBlendStateIR::ColorComponent::BLUE
                        },
                        {
                            ColorBlendConfiguration::ColorComponent::ALPHA,
                            ColorBlendStateIR::ColorComponent::ALPHA
                        },
                    };

            return createColorBlendStateGir(configuration,
                                            LOGIC_OPERATION_CONVERSION_MAP,
                                            BLEND_FACTOR_CONVERSION_MAP,
                                            BLEND_OPERATION_CONVERSION_MAP,
                                            COLOR_COMPONENT_CONVERSION_MAP);
        }

        static DepthStencilStateIR
        bakeDepthStencilState(const DepthStencilConfiguration &configuration) {
            std::unordered_map<
                DepthStencilConfiguration::CompareOperation,
                DepthStencilStateIR::CompareOperation
            > COMPARE_OPERATION_CONVERSION_MAP
                    = {
                        {
                            DepthStencilConfiguration::CompareOperation::NO_OP,
                            DepthStencilStateIR::CompareOperation::NO_OP
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::NEVER,
                            DepthStencilStateIR::CompareOperation::NEVER
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::LESS,
                            DepthStencilStateIR::CompareOperation::LESS
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::EQUAL,
                            DepthStencilStateIR::CompareOperation::EQUAL
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::LESS_OR_EQUAL,
                            DepthStencilStateIR::CompareOperation::LESS_OR_EQUAL
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::GREATER,
                            DepthStencilStateIR::CompareOperation::GREATER
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::NOT_EQUAL,
                            DepthStencilStateIR::CompareOperation::NOT_EQUAL
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::GREATER_OR_EQUAL,
                            DepthStencilStateIR::CompareOperation::GREATER_OR_EQUAL
                        },
                        {
                            DepthStencilConfiguration::CompareOperation::ALWAYS,
                            DepthStencilStateIR::CompareOperation::ALWAYS
                        },
                    };
            std::unordered_map<
                DepthStencilConfiguration::StencilOperation,
                DepthStencilStateIR::StencilOperation
            > STENCIL_OPERATION_CONVERSION_MAP
                    = {
                        {
                            DepthStencilConfiguration::StencilOperation::NO_OP,
                            DepthStencilStateIR::StencilOperation::NO_OP
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::KEEP,
                            DepthStencilStateIR::StencilOperation::KEEP
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::ZERO,
                            DepthStencilStateIR::StencilOperation::ZERO
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::REPLACE,
                            DepthStencilStateIR::StencilOperation::REPLACE
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::INCREMENT_AND_CLAMP,
                            DepthStencilStateIR::StencilOperation::INCREMENT_AND_CLAMP
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::DECREMENT_AND_CLAMP,
                            DepthStencilStateIR::StencilOperation::DECREMENT_AND_CLAMP
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::INVERT,
                            DepthStencilStateIR::StencilOperation::INVERT
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::INCREMENT_AND_WRAP,
                            DepthStencilStateIR::StencilOperation::INCREMENT_AND_WRAP
                        },
                        {
                            DepthStencilConfiguration::StencilOperation::DECREMENT_AND_WRAP,
                            DepthStencilStateIR::StencilOperation::DECREMENT_AND_WRAP
                        },
                    };

            return createDepthStencilStateGir(configuration,
                                              COMPARE_OPERATION_CONVERSION_MAP,
                                              STENCIL_OPERATION_CONVERSION_MAP);
        }

        static DynamicStateIR bakeDynamicState(const DynamicStateConfiguration &configuration) {
            static const std::unordered_map<
                DynamicStateConfiguration::DynamicStateSource,
                DynamicStateIR::DynamicStateSource
            > DYNAMIC_STATE_CONVERSION_MAP
                    = {
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VIEWPORT,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_VIEWPORT
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_SCISSOR,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_SCISSOR
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_LINE_WIDTH,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_LINE_WIDTH
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BIAS,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BIAS
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_BLEND_CONSTANTS,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_BLEND_CONSTANTS
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_COMPARE_MASK,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_COMPARE_MASK
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_WRITE_MASK,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_WRITE_MASK
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_REFERENCE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_REFERENCE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_CULL_MODE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_CULL_MODE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_FRONT_FACE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_FRONT_FACE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_PRIMITIVE_TOPOLOGY,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_PRIMITIVE_TOPOLOGY
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VIEWPORT_WITH_COUNT,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_VIEWPORT_WITH_COUNT
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_SCISSOR_WITH_COUNT,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_SCISSOR_WITH_COUNT
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_VERTEX_INPUT_BINDING_STRIDE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_TEST_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_TEST_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_WRITE_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_WRITE_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_COMPARE_OP,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_COMPARE_OP
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_TEST_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_TEST_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_STENCIL_OP,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_STENCIL_OP
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_RASTERIZER_DISCARD_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_RASTERIZER_DISCARD_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_DEPTH_BIAS_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_DEPTH_BIAS_ENABLE
                        },
                        {
                            DynamicStateConfiguration::DynamicStateSource::DYNAMIC_PRIMITIVE_RESTART_ENABLE,
                            DynamicStateIR::DynamicStateSource::DYNAMIC_PRIMITIVE_RESTART_ENABLE
                        },
                    };

            return createDynamicStateGir(configuration, DYNAMIC_STATE_CONVERSION_MAP);
        }

        static MultisampleStateIR bakeMultisampleState(const MultisampleConfiguration &configuration) {
            static const std::unordered_map<
                MultisampleConfiguration::NumberOfSamples,
                MultisampleStateIR::NumberOfSamples
            > NUMBER_OF_SAMPLES_CONVERSION_MAP
                    = {
                        {
                            MultisampleConfiguration::NumberOfSamples::UNSET,
                            MultisampleStateIR::NumberOfSamples::UNSET
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::ONE,
                            MultisampleStateIR::NumberOfSamples::ONE
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::TWO,
                            MultisampleStateIR::NumberOfSamples::TWO
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::FOUR,
                            MultisampleStateIR::NumberOfSamples::FOUR
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::EIGHT,
                            MultisampleStateIR::NumberOfSamples::EIGHT
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::SIXTEEN,
                            MultisampleStateIR::NumberOfSamples::SIXTEEN
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::THIRTY_TWO,
                            MultisampleStateIR::NumberOfSamples::THIRTY_TWO
                        },
                        {
                            MultisampleConfiguration::NumberOfSamples::SIXTY_FOUR,
                            MultisampleStateIR::NumberOfSamples::SIXTY_FOUR
                        },
                    };

            return createMultisampleStateGir(configuration, NUMBER_OF_SAMPLES_CONVERSION_MAP);
        }

        static PrimitiveAssemblyIR
        bakePrimitiveAssemblyState(const PrimitiveAssemblyConfiguration &configuration) {
            static const std::unordered_map<
                PrimitiveAssemblyConfiguration::PrimitiveTopology,
                PrimitiveAssemblyIR::PrimitiveTopology
            > PRIMITIVE_TOPOLOGY_CONVERSION_MAP = {
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::POINT_LIST,
                    PrimitiveAssemblyIR::PrimitiveTopology::POINT_LIST
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_LIST,
                    PrimitiveAssemblyIR::PrimitiveTopology::LINE_LIST
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_STRIP,
                    PrimitiveAssemblyIR::PrimitiveTopology::LINE_STRIP
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_LIST,
                    PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_LIST
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_STRIP,
                    PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_STRIP
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_FAN,
                    PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_FAN
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY,
                    PrimitiveAssemblyIR::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY,
                    PrimitiveAssemblyIR::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY,
                    PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY,
                    PrimitiveAssemblyIR::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY
                },
                {
                    PrimitiveAssemblyConfiguration::PrimitiveTopology::PATCH_LIST,
                    PrimitiveAssemblyIR::PrimitiveTopology::PATCH_LIST
                },
            };

            return createPrimitiveAssemblyGir(configuration, PRIMITIVE_TOPOLOGY_CONVERSION_MAP);
        }

        static RasterizationStateIR
        bakeRasterizationState(const RasterizationConfiguration &configuration) {
            static const std::unordered_map<
                RasterizationConfiguration::PolygonRasterizationMode,
                RasterizationStateIR::PolygonRasterizationMode
            > POLYGON_RASTERIZATION_MODE_CONVERSION_MAP = {
                {
                    RasterizationConfiguration::PolygonRasterizationMode::FILL,
                    RasterizationStateIR::PolygonRasterizationMode::FILL
                },
                {
                    RasterizationConfiguration::PolygonRasterizationMode::POINTS,
                    RasterizationStateIR::PolygonRasterizationMode::POINTS
                },
                {
                    RasterizationConfiguration::PolygonRasterizationMode::LINES,
                    RasterizationStateIR::PolygonRasterizationMode::LINES
                },
            };
            static const std::unordered_map<
                RasterizationConfiguration::PolygonCullingMode,
                RasterizationStateIR::PolygonCullingMode
            > POLYGON_CULLING_MODE_CONVERSION_MAP = {
                {
                    RasterizationConfiguration::PolygonCullingMode::NONE,
                    RasterizationStateIR::PolygonCullingMode::NONE
                },
                {
                    RasterizationConfiguration::PolygonCullingMode::FRONT_FACING,
                    RasterizationStateIR::PolygonCullingMode::FRONT_FACING
                },
                {
                    RasterizationConfiguration::PolygonCullingMode::BACK_FACING,
                    RasterizationStateIR::PolygonCullingMode::BACK_FACING
                },
                {
                    RasterizationConfiguration::PolygonCullingMode::ALL,
                    RasterizationStateIR::PolygonCullingMode::ALL
                },
            };
            static const std::unordered_map<
                RasterizationConfiguration::PolygonFrontFaceOrientationMode,
                RasterizationStateIR::PolygonFrontFaceOrientationMode
            > POLYGON_FRONT_FACE_ORIENTATION_CONVERSION_MAP = {
                {
                    RasterizationConfiguration::PolygonFrontFaceOrientationMode::CLOCKWISE,
                    RasterizationStateIR::PolygonFrontFaceOrientationMode::CLOCKWISE
                },
                {
                    RasterizationConfiguration::PolygonFrontFaceOrientationMode::COUNTER_CLOCKWISE,
                    RasterizationStateIR::PolygonFrontFaceOrientationMode::COUNTER_CLOCKWISE
                },
            };

            return createRasterizationStateGir(configuration,
                                               POLYGON_RASTERIZATION_MODE_CONVERSION_MAP,
                                               POLYGON_CULLING_MODE_CONVERSION_MAP,
                                               POLYGON_FRONT_FACE_ORIENTATION_CONVERSION_MAP);
        }

        static TessellationStateIR
        bakeTessellationState(const TessellationConfiguration &configuration) {
            return {
                configuration.enableTessellation,
                configuration.numberOfPatchControlPoints
            };
        }

        static VertexInputStateIR bakeVertexInputState(const VertexInputConfiguration &configuration) {
            return {

            };
        }

        /**
         * Given a list of scene resource GIR objects, this returns all BufferIRs within it.
         * @param sceneResourceGirs
         * @return The buffers
         */
        [[nodiscard]] static std::vector<gir::BufferIR>
        obtainBufferGirsFromListOfAllResourceGirs(
            const std::vector<gir::GraphicsIntermediateRepresentation> &sceneResourceGirs) {
            // copy only the buffer resources into the bufferGirs list
            std::vector<gir::GraphicsIntermediateRepresentation> bufferGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(bufferGirs),
                         [&](const gir::GraphicsIntermediateRepresentation &sceneResource) {
                             // copy element if its renderPassSubtype denotes it as a buffer
                             return sceneResource.subtype == gir::GIRSubtype::BUFFER;
                         }
            );
            // cast the bufferGirs to their actual pointer type (BufferIR)
            std::vector<gir::BufferIR> convertedBufferGirs = {};
            std::transform(bufferGirs.begin(), bufferGirs.end(), std::back_inserter(convertedBufferGirs),
                           [&](const gir::GraphicsIntermediateRepresentation &gir) {
                               // cast the shared pointer to BufferIR
                               // TODO -> finish factoring out these stupid shared pointers that don't need to exist
                               return *(dynamic_cast<const gir::BufferIR *>(&gir));
                           });
            return convertedBufferGirs;
        }

        // IMAGE BAKE HELPER FUNCTIONS

        /**
         * Alright this ancient old function might be something I can adapt for
         * the ever-changing needs of the engine.
         *
         * Big issue with it: it's based around having a pre-baked list of images;
         * good news is that it does seem to spit out the gir::renderPass::ImageAttachments I
         * was hoping already existed lol.
         *
         * *For now*, that's okay because I'm just doing a janky thing where the graph
         * is linear off the bat and we can just add more jank by gathering the resources
         * by iterating over all the passes and their various resources.
         *
         * When the scene graph refactor comes, I think it wouldn't be that hard to adapt it to instead take only the image attachment
         * list and bake the image for the first time here. This way it might be able to
         * survive and be used when we're baking the graph by traversing it bottom-up
         * and building resources the first time we see them (at least i think that's how it'll go rn)
         *
         * @param sceneImageAttachmentList
         * @param allBakedImageGirs
         * @return
         */
        static std::vector<gir::renderPass::ImageAttachment>
        bakeListOfSceneImageAttachments(std::vector<ImageAttachment> &sceneImageAttachmentList,
                                        std::vector<gir::ImageIR> &allBakedImageGirs) {
            // build list of image attachment IR
            std::vector<gir::renderPass::ImageAttachment> imageAttachmentGirs = {};
            for (ImageAttachment &imageAttachment: sceneImageAttachmentList) {
                // first obtain only the images that are attached to this render pass through the given attachment list
                gir::ImageIR *attachedImage = nullptr;
                for (gir::ImageIR &imageGir: allBakedImageGirs) {
                    if (imageGir.uid == imageAttachment.imageIdentifier) {
                        attachedImage = &imageGir;
                        break;
                    }
                }
                if (!attachedImage) {
                    // TODO -> log that this happened!
                    return {};
                }

                // // FOR NOW -> swapchain images will be baked without prompting from the bake output
                if (attachedImage->imageUsage == gir::ImageIR::ImageUsage::SWAPCHAIN_IMAGE) {
                    continue;
                }

                imageAttachmentGirs.push_back(
                    {
                        attachedImage->uid,
                        (attachedImage->imageFormat),
                        convertImageAttachmentState(imageAttachment.imageState),
                        convertImageAttachmentMultisampleResolveOperation(imageAttachment.resolveOperation),
                        convertImageAttachmentResolveState(imageAttachment.resolveState),
                        getImageAttachmentLoadOperation(imageAttachment.loadOperation),
                        getImageAttachmentStoreOperation(imageAttachment.storeOperation),
                        getImageAttachmentClearValue(imageAttachment.clearValue)
                    }
                );
            }
            return imageAttachmentGirs;
        }

        /**
         *
         *
         * @param imageState
         * @return
         */
        static gir::renderPass::AttachmentState
        convertImageAttachmentState(const AttachmentState imageState) {
            static const std::unordered_map<AttachmentState, gir::renderPass::AttachmentState>
                    conversionMap = {
                        {
                            AttachmentState::UNDEFINED,
                            gir::renderPass::AttachmentState::UNDEFINED
                        },
                        {
                            AttachmentState::PREINITIALIZED,
                            gir::renderPass::AttachmentState::PREINITIALIZED
                        },
                        {
                            AttachmentState::COLOR_ATTACHMENT,
                            gir::renderPass::AttachmentState::COLOR_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_ATTACHMENT
                        },
                        {
                            AttachmentState::STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_ATTACHMENT
                        },
                        {
                            AttachmentState::TRANSFER_SOURCE,
                            gir::renderPass::AttachmentState::TRANSFER_SOURCE
                        },
                        {
                            AttachmentState::TRANSFER_DESTINATION,
                            gir::renderPass::AttachmentState::TRANSFER_DESTINATION
                        },
                        {
                            AttachmentState::SWAPCHAIN_COLOR_ATTACHMENT,
                            gir::renderPass::AttachmentState::SWAPCHAIN_COLOR_ATTACHMENT
                        },
                    };

            return conversionMap.at(imageState);
        }

        /**
         *
         * @param resolveOperation
         * @return
         */
        static gir::renderPass::MultisampleResolveOperation
        convertImageAttachmentMultisampleResolveOperation(const MultisampleResolveOperation resolveOperation) {
            static const std::unordered_map<MultisampleResolveOperation,
                        gir::renderPass::MultisampleResolveOperation>
                    conversionMap = {
                        {
                            MultisampleResolveOperation::NO_OP,
                            gir::renderPass::MultisampleResolveOperation::NO_OP
                        },
                        {
                            MultisampleResolveOperation::SET_TO_FIRST_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::SET_TO_FIRST_SAMPLE
                        },
                        {
                            MultisampleResolveOperation::AVERAGE_OF_ALL_SAMPLES,
                            gir::renderPass::MultisampleResolveOperation::AVERAGE_OF_ALL_SAMPLES
                        },
                        {
                            MultisampleResolveOperation::MINIMUM_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::MINIMUM_SAMPLE
                        },
                        {
                            MultisampleResolveOperation::MAXIMUM_SAMPLE,
                            gir::renderPass::MultisampleResolveOperation::MAXIMUM_SAMPLE
                        }
                    };

            return conversionMap.at(resolveOperation);
        }

        /**
         *
         * @return
         */
        static gir::renderPass::AttachmentState convertImageAttachmentResolveState(const AttachmentState resolveState) {
            static const std::unordered_map<AttachmentState, gir::renderPass::AttachmentState>
                    conversionMap = {
                        {
                            AttachmentState::UNDEFINED,
                            gir::renderPass::AttachmentState::UNDEFINED
                        },
                        {
                            AttachmentState::PREINITIALIZED,
                            gir::renderPass::AttachmentState::PREINITIALIZED
                        },
                        {
                            AttachmentState::COLOR_ATTACHMENT,
                            gir::renderPass::AttachmentState::COLOR_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_STENCIL_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::DEPTH_ATTACHMENT,
                            gir::renderPass::AttachmentState::DEPTH_ATTACHMENT
                        },
                        {
                            AttachmentState::STENCIL_READ_ONLY_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_READ_ONLY_ATTACHMENT
                        },
                        {
                            AttachmentState::STENCIL_ATTACHMENT,
                            gir::renderPass::AttachmentState::STENCIL_ATTACHMENT
                        },
                        {
                            AttachmentState::TRANSFER_SOURCE,
                            gir::renderPass::AttachmentState::TRANSFER_SOURCE
                        },
                        {
                            AttachmentState::TRANSFER_DESTINATION,
                            gir::renderPass::AttachmentState::TRANSFER_DESTINATION
                        },
                    };

            return conversionMap.at(resolveState);
        }

        /**
         *
         * @return
         */
        static gir::renderPass::AttachmentLoadOperation
        getImageAttachmentLoadOperation(const AttachmentLoadOperation attachmentLoadOperation) {
            static const std::unordered_map<AttachmentLoadOperation,
                gir::renderPass::AttachmentLoadOperation> conversionMap = {
                {
                    AttachmentLoadOperation::DONT_CARE,
                    gir::renderPass::AttachmentLoadOperation::DONT_CARE
                },
                {
                    AttachmentLoadOperation::LOAD,
                    gir::renderPass::AttachmentLoadOperation::LOAD
                },
                {
                    AttachmentLoadOperation::CLEAR,
                    gir::renderPass::AttachmentLoadOperation::CLEAR
                }
            };

            return conversionMap.at(attachmentLoadOperation);
        }

        /**
         *
         * @return
         */
        static gir::renderPass::AttachmentStoreOperation
        getImageAttachmentStoreOperation(const AttachmentStoreOperation attachmentStoreOperation) {
            static const std::unordered_map<AttachmentStoreOperation,
                gir::renderPass::AttachmentStoreOperation> conversionMap = {
                {
                    AttachmentStoreOperation::DONT_CARE,
                    gir::renderPass::AttachmentStoreOperation::DONT_CARE
                },
                {
                    AttachmentStoreOperation::STORE,
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
            const std::vector<gir::ImageIR> &attachedImageGirs
        ) {
            std::unordered_map<UniqueIdentifier, unsigned> attachedImageGirIndices = {};
            int index = 0;
            for (const auto &attachedImageGir: attachedImageGirs) {
                attachedImageGirIndices[attachedImageGir.uid] = index;
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
        static std::vector<gir::ImageIR> obtainImageGirsFromListOfAllResourceGirs(
            const std::vector<gir::GraphicsIntermediateRepresentation> &sceneResourceGirs) {
            // copy only the images into the imageGirs list
            std::vector<gir::GraphicsIntermediateRepresentation> imageGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(imageGirs),
                         [&](const gir::GraphicsIntermediateRepresentation &sceneResourceGir) {
                             return sceneResourceGir.subtype == gir::GIRSubtype::IMAGE;
                         });
            // cast the imageGirs to their actual pointer type (ImageIR)
            std::vector<gir::ImageIR> convertedImageGirs = {};
            std::transform(imageGirs.begin(), imageGirs.end(), std::back_inserter(convertedImageGirs),
                           [&](const gir::GraphicsIntermediateRepresentation &gir) {
                               // cast the shared pointer to ImageIR
                               return *dynamic_cast<const gir::ImageIR *>(&gir);
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
        static gir::ImageIR
        findTextureImageIRFromList(const std::vector<gir::ImageIR> &imageGirs,
                                   const TextureAttachment &attachment) {
            const auto &findAttachedImageIterator = std::find_if(imageGirs.begin(), imageGirs.end(),
                                                                 [&](const gir::ImageIR &imageGir) {
                                                                     return imageGir.uid ==
                                                                            attachment.textureResource->getUid();
                                                                 });
            if (findAttachedImageIterator == imageGirs.end()) {
                return {};
            }

            return *findAttachedImageIterator;
        }

        /**
         *
         * @param sceneShaderStages
         * @return
         */
        static std::vector<gir::renderPass::TextureAttachment::ShaderStage>
        convertShaderStages(
            const std::vector<TextureAttachment::ShaderStage> &sceneShaderStages) {
            // TODO - replace this 2-stage "implementation" with an actual conversion map (or whichever other method)
            std::vector<gir::renderPass::TextureAttachment::ShaderStage> shaderStages = {};
            for (const auto &sceneShaderStage: sceneShaderStages) {
                gir::renderPass::TextureAttachment::ShaderStage convertedShaderStage
                        = (sceneShaderStage ==
                           TextureAttachment::ShaderStage::VERTEX)
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
        static std::vector<gir::ShaderConstantIR> obtainShaderConstantGirsFromListOfAllResourceGirs(
            const std::vector<gir::GraphicsIntermediateRepresentation> &sceneResourceGirs) {
            // copy only the shader constants into the shaderConstantGirs list
            std::vector<gir::GraphicsIntermediateRepresentation> shaderConstantGirs = {};
            std::copy_if(sceneResourceGirs.begin(), sceneResourceGirs.end(), std::back_inserter(shaderConstantGirs),
                         [&](const gir::GraphicsIntermediateRepresentation &sceneResourceGir) {
                             return sceneResourceGir.subtype == gir::GIRSubtype::SHADER_CONSTANT;
                         });
            // cast the shaderConstantGirs to their actual pointer type (ShaderConstantIR)
            std::vector<gir::ShaderConstantIR> convertedShaderConstantGirs = {};
            std::transform(shaderConstantGirs.begin(), shaderConstantGirs.end(),
                           std::back_inserter(convertedShaderConstantGirs),
                           [&](const gir::GraphicsIntermediateRepresentation &gir) {
                               return *dynamic_cast<const gir::ShaderConstantIR *>(&gir);
                           });
            return convertedShaderConstantGirs;
        }

        // BUFFER BAKE HELPER FUNCTIONS

        static std::vector<gir::renderPass::BufferAttachment>
        bakeListOfBufferAttachments(const std::vector<Buffer> &bufferAttachments,
                                    const std::vector<gir::BufferIR> &sceneBufferGirs) {
            std::vector<gir::renderPass::BufferAttachment> bufferAttachmentGirs = {};

            for (const auto &attachment: bufferAttachments) {
                const auto &attachedBufferGirItr
                        = std::find_if(sceneBufferGirs.begin(), sceneBufferGirs.end(),
                                       [&](const gir::BufferIR &buffer) {
                                           return buffer.uid == attachment.uid;
                                       });

                if (attachedBufferGirItr == sceneBufferGirs.end()) {
                    // TODO - better logging
                    throw std::runtime_error(
                        "Error in scene::RenderPass::bakeListOfBufferAttachments() - attached buffer not found!");
                }

                bufferAttachmentGirs.push_back(
                    gir::renderPass::BufferAttachment{
                        *(attachedBufferGirItr)
                    }
                );
            }

            return bufferAttachmentGirs;
        }

        // SHADER ATTACHMENT BAKE HELPER FUNCTIONS

        [[nodiscard]] static std::vector<gir::ShaderModuleIR> obtainShaderModuleGirsList(
            const std::vector<gir::GraphicsIntermediateRepresentation> &sceneShaderModuleGirs) {
            std::vector<gir::ShaderModuleIR> sceneShaderModules = {};
            std::transform(sceneShaderModuleGirs.begin(), sceneShaderModuleGirs.end(),
                           std::back_inserter(sceneShaderModules),
                           [&](const gir::GraphicsIntermediateRepresentation &sceneShaderModuleGir) {
                               return *dynamic_cast<const gir::ShaderModuleIR *>(&sceneShaderModuleGir);
                           });

            return sceneShaderModules;
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
            const std::unordered_map<ColorBlendConfiguration::LogicOperation,
                ColorBlendStateIR::LogicOperation> &logicOperationConversionMap,
            const std::unordered_map<ColorBlendConfiguration::BlendFactor,
                ColorBlendStateIR::BlendFactor> &blendFactorConversionMap,
            const std::unordered_map<ColorBlendConfiguration::BlendOperation,
                ColorBlendStateIR::BlendOperation> &blendOperationConversionMap,
            const std::unordered_map<ColorBlendConfiguration::ColorComponent,
                ColorBlendStateIR::ColorComponent> &colorComponentConversionMap) {
            std::vector<ColorBlendStateIR::ColorAttachmentBlendState> colorAttachmentBlendStateGirs
                    = bakePipelineColorAttachmentBlendStateGirs(
                        configuration,
                        blendFactorConversionMap,
                        blendOperationConversionMap,
                        colorComponentConversionMap);

            return ColorBlendStateIR{
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
            const std::unordered_map<ColorBlendConfiguration::BlendFactor,
                ColorBlendStateIR::BlendFactor> &blendFactorConversionMap,
            const std::unordered_map<ColorBlendConfiguration::BlendOperation,
                ColorBlendStateIR::BlendOperation> &blendOperationConversionMap,
            const std::unordered_map<ColorBlendConfiguration::ColorComponent,
                ColorBlendStateIR::ColorComponent> &colorComponentConversionMap) {
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
                configuration.depthBoundMaximumValue,
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
                                                                    RasterizationConfiguration::PolygonRasterizationMode
                                                                    , RasterizationStateIR::PolygonRasterizationMode> &
                                                                rasterizationModeConversionMap,
                                                                const std::unordered_map<
                                                                    RasterizationConfiguration::PolygonCullingMode,
                                                                    RasterizationStateIR::PolygonCullingMode> &
                                                                polygonCullingModeConversionMap,
                                                                const std::unordered_map<
                                                                    RasterizationConfiguration::PolygonFrontFaceOrientationMode
                                                                    , RasterizationStateIR::PolygonFrontFaceOrientationMode>
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
