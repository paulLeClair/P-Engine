#pragma once

#include "../GraphicsIntermediateRepresentation.hpp"
#include "../ResourceIR/BufferIR/BufferIR.hpp"
#include "../VertexAttributeIR/VertexAttributeIR.hpp"

#include "../../../utilities/Hash/Hash.hpp"
#include "../model/AnimationIR/AnimationIR.hpp"

namespace pEngine::girEngine::gir {
    struct VertexBufferAttachment {
        VertexBufferAttachment(const BufferIR &attachedBuffer, unsigned int vertexCount)
            : attachedBuffer(attachedBuffer), vertexCount(vertexCount) {
        }

        BufferIR attachedBuffer = {};
        unsigned vertexCount = 0;
        // any other info we want to include for 1 buffer binding can go here;
        // note that each vertex buffer must be compatible with the target vertex
        // input binding for a particular draw attachment
    };

    struct IndexBufferAttachment {
        IndexBufferAttachment(const BufferIR &attachedBuffer, unsigned int indexCount)
            : attachedBuffer(attachedBuffer), indexCount(indexCount) {
        }

        BufferIR attachedBuffer = {};
        unsigned indexCount = 0;
    };

    struct MeshAttachment {
        // issue: when we're passing the info in from the gir, we don't want to be
        // using these unwrapped buffers only because they can't store additional data
        // without pollution.
        // So instead I'll replace them with a simple wrapper that can then include
        // any other auxiliary info that we need as the engine matures
        std::vector<VertexBufferAttachment> vertexBuffers;
        std::vector<IndexBufferAttachment> indexBuffers;

        // TODO -> materials

        // NOTE: engine-native anims are currently disabled
        boost::optional<model::AnimationIR> animation = boost::none;
    };

    /**
     * Instead of my pre-existing idea for this whole thing, I'm going to try and
     * unify geometry and draw command representation and make the overall interface as simple as possible.\n\n
     *
     * With the new front-end/gir geometry stuff that I'm gonna add, this will probably become what
     * our BoundGeometry class should bake to; I'll rename it actually to be a bit more logical
    */
    struct DrawAttachmentIR : GraphicsIntermediateRepresentation {
        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            /**
             * This specifies the index of the geometry binding GIR (which needs to be created atow)
             * that this particular draw attachment corresponds to within a particular render pass
             * (which is the thing that it is "attached" to)
             */
            unsigned vertexInputBindingIndex = 0;


            std::vector<MeshAttachment> meshAttachments;
        };

        explicit DrawAttachmentIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              meshAttachments(creationInput.meshAttachments) {
        }

        DrawAttachmentIR() : DrawAttachmentIR(CreationInput()) {
        }

        ~DrawAttachmentIR() override = default;

        unsigned vertexBindingIndex = 0;
        std::vector<MeshAttachment> meshAttachments;
    };
}
