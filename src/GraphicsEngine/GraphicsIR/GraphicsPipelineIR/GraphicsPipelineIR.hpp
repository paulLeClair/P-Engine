#pragma once

#include <utility>

#include "../GraphicsIntermediateRepresentation.hpp"
#include "ColorBlendStateIR/ColorBlendStateIR.hpp"
#include "DepthStencilStateIR/DepthStencilStateIR.hpp"
#include "DynamicStateIR/DynamicStateIR.hpp"
#include "MultisampleStateIR/MultisampleStateIR.hpp"
#include "PrimitiveAssemblyIR/PrimitiveAssemblyIR.hpp"
#include "RasterizationStateIR/RasterizationStateIR.hpp"
#include "TessellationStateIR/TessellationStateIR.hpp"
#include "VertexInputStateIR/VertexInputStateIR.hpp"

using namespace pEngine::girEngine::gir::pipeline;

namespace pEngine::girEngine::gir {

/**
 * This one's a bit more involved than a lot of the other ones;
 * 
 * It probably would pay to do a bit more fleshing out of this one 
 * but honestly implementing these is probably best done in a later issue...
 * 
 * However, we can at least add stub classes for the basic chunks of information that
 * the graphics pipeline IR will need to store in order to build the Vulkan graphics pipeline
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
                  vertexInputState(creationInput.vertexInputState) {
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
    };

}