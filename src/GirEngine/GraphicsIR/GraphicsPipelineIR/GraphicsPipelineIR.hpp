#pragma once

#include <utility>
#include <boost/optional.hpp>

#include "../GraphicsIntermediateRepresentation.hpp"
#include "ColorBlendStateIR/ColorBlendStateIR.hpp"
#include "DepthStencilStateIR/DepthStencilStateIR.hpp"
#include "DynamicStateIR/DynamicStateIR.hpp"
#include "MultisampleStateIR/MultisampleStateIR.hpp"
#include "PrimitiveAssemblyIR/PrimitiveAssemblyIR.hpp"
#include "RasterizationStateIR/RasterizationStateIR.hpp"
#include "TessellationStateIR/TessellationStateIR.hpp"
#include "VertexInputStateIR/VertexInputStateIR.hpp"
#include "DynamicRenderPassPipelineStateIR/DynamicRenderPassPipelineStateIR.hpp"

using namespace pEngine::girEngine::gir::pipeline;

namespace pEngine::girEngine::gir {
    /**
      * YO - I'm fairly sure this thing isn't getting used anywhere;
      * TODO -> wire this in to the render pass girs
      *
     */
    class GraphicsPipelineIR : public GraphicsIntermediateRepresentation {
    public:
        /**
         * This one is going to take some iteration- it should contain all the
         * required state to be able to build a full graphics pipeline.
         *
         * To start, I'll just include the existing stuff that's ripped right out of Vulkan.
         * Going forward, until we make significant progress with the Vulkan-only stuff,
         * I'll just tailor this to be everything required to make an abstraction of a
         * Vulkan graphics pipeline being used as part of some render pass.
         *
         * This might mean tying things together which Vulkan ties together (eg
         * all the stuff that a command executed as part of a render pass has to
         * bind), but I like the idea of coming back and adapting it later so that
         * it can be extended to work with other backends than Vulkan.
         *
         */
        struct CreationInput : public GraphicsIntermediateRepresentation::CreationInput {
            ColorBlendStateIR colorBlendState;
            DepthStencilStateIR depthStencilState;
            DynamicStateIR dynamicState;
            MultisampleStateIR multisampleState;
            PrimitiveAssemblyIR primitiveAssemblyState;
            RasterizationStateIR rasterizationState;
            TessellationStateIR tessellationState;
            VertexInputStateIR vertexInputState;
            boost::optional<DynamicRenderPassPipelineStateIR> dynamicRenderPassState = boost::none;
        };

        explicit GraphicsPipelineIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              colorBlendState(creationInput.colorBlendState),
              depthStencilState(creationInput.depthStencilState),
              dynamicState(creationInput.dynamicState),
              multisampleState(creationInput.multisampleState),
              primitiveAssemblyState(creationInput.primitiveAssemblyState),
              rasterizationState(creationInput.rasterizationState),
              tessellationState(creationInput.tessellationState),
              vertexInputState(creationInput.vertexInputState),
              dynamicRenderPassState(creationInput.dynamicRenderPassState) {
        }

        GraphicsPipelineIR() : GraphicsIntermediateRepresentation({}) {
        }

        ~GraphicsPipelineIR() override = default;

    private:
        ColorBlendStateIR colorBlendState;
        DepthStencilStateIR depthStencilState;
        DynamicStateIR dynamicState;
        MultisampleStateIR multisampleState;
        PrimitiveAssemblyIR primitiveAssemblyState;
        RasterizationStateIR rasterizationState;
        TessellationStateIR tessellationState;
        VertexInputStateIR vertexInputState;
        boost::optional<DynamicRenderPassPipelineStateIR> dynamicRenderPassState = boost::none;
    };
}
