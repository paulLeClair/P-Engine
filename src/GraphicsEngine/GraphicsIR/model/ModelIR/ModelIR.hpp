//
// Created by paull on 2024-09-03.
//

#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../AnimationIR/AnimationIR.hpp"
#include "../../DrawAttachmentIR/DrawAttachmentIR.hpp"

#include "../../../scene/SceneSpace/Position/Position.hpp"
#include "../../../scene/SceneSpace/Orientation/Orientation.hpp"

namespace pEngine::girEngine::gir::model {
    /**
     * Ugh right off the bat: I hate how the "l" and the "IR" kinda blend together on sans-serif fonts lol
     *
     * The idea with these is that they should be centered around the draw attachment IR classes;
     */
    struct ModelIR : public GraphicsIntermediateRepresentation {
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
            std::vector<BufferIR> buffers = {};
            std::vector<DrawAttachmentIR> drawAttachments = {};
            scene::space::position::Position &position;
            scene::space::orient::Orientation &orientation;
        };

        explicit ModelIR(const CreationInput &input)
            : GraphicsIntermediateRepresentation(
                  GraphicsIntermediateRepresentation::CreationInput{
                      input.name,
                      input.uid,
                      GIRSubtype::MODEL
                  }),
              drawAttachments(input.drawAttachments),
              buffers(input.buffers),
              position(input.position),
              orientation(input.orientation) {
        }

        std::vector<DrawAttachmentIR> drawAttachments = {};

        // animation ir
        boost::optional<AnimationIR> animation = boost::none;

        std::vector<BufferIR> buffers = {};

        // temporary hacky mechanism to label animation data for the system
        scene::space::position::Position &position;
        scene::space::orient::Orientation &orientation;

        // TODO - other resource attachments
    };
}
