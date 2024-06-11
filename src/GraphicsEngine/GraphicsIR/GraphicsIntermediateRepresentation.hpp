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
        BUFFER,
        IMAGE,
        SHADER_CONSTANT,
        TEXTURE,
        SHADER_MODULE
    };

    /**
     * This should be the top-level class that all IR inherits from;
     * I'm not sure exactly what will be part of this but probably
     * we can start with some basic stuff.\n\n
     *
     * A lot of this will be redone anyway once we implement the backend side of things
     * and after that as well.\n\n
     *
     * One consideration: it may make sense to define a "GIRSet" type struct which
     * just groups up all the different gear types by renderPassSubtype and ensures that the backend
     * would just be able to go through the struct. Maybe that would be better done as a refactoring
     * when we get to the backend stuff, especially since it wouldn't be that hard to adapt things
     * from the current idea of just batching all gir into one big list for processing.
     */
    class GraphicsIntermediateRepresentation {
    public:
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

        GraphicsIntermediateRepresentation(const GraphicsIntermediateRepresentation &other) = default;

        virtual ~GraphicsIntermediateRepresentation() = default;

        [[maybe_unused]] [[nodiscard]] GIRSubtype getSubtype() const {
            return subtype;
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uid;
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;
        GIRSubtype subtype;
    };
}
