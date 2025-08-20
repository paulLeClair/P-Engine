//
// Created by paull on 2023-09-04.
//

#pragma once

#include "../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::gir {
    enum class GIRSubtype {
        UNKNOWN,
        DRAW_COMMANDS,
        RENDER_GRAPH,
        RENDER_PASS,
        GRAPHICS_PIPELINE,
        MODEL,
        ANIMATION,
        KEYFRAME,
        BUFFER,
        IMAGE,
        SHADER_CONSTANT,
        TEXTURE,
        SHADER_MODULE,
        CAMERA,
        VERTEX_INPUT_BINDING
    };

    struct GraphicsIntermediateRepresentation {
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
            GIRSubtype subtype;
        };

        explicit GraphicsIntermediateRepresentation(const CreationInput &creationInput)
                : name(creationInput.name),
                  uid(creationInput.uid),
                  subtype(creationInput.subtype) {
        }

        GraphicsIntermediateRepresentation() = default;

        GraphicsIntermediateRepresentation(const GraphicsIntermediateRepresentation &other) = default;

        virtual ~GraphicsIntermediateRepresentation() = default;

        std::string name;
        util::UniqueIdentifier uid;
        GIRSubtype subtype = GIRSubtype::UNKNOWN;
    };
}
