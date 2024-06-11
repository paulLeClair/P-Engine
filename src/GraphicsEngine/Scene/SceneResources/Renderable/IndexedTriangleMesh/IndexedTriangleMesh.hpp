//
// Created by paull on 2022-12-12.
//

#pragma once

#include <memory>
#include "../Renderable.hpp"
#include "../../../Scene.hpp"

#include "../../Buffer/IndexBuffer/IndexBuffer.hpp"
#include "../../Buffer/VertexBuffer/VertexBuffer.hpp"


namespace pEngine::girEngine::scene {

    /**
     * This will be one of the first built-in high-level geometry representations and probably also one of the most commonly used,
     * since it's natively supported by basically every graphics card (although I know nothing about ray-tracing architecture and
     * that sort of thing - will be something for a future engine iteration!)
     *
     * The idea is that we should be able to just standardize a representation for indexed meshes at a high level,
     * and then the user can just hand it off to the engine and expect the appropriate data structures to be constructed in the backend
     * either at bake time or as part of a per-frame update.
     *
     * We already have a standardized Vertices class started, so we'll use an Indices class as well. This also makes it so that
     * potentially we can break each of them apart into an abstract class and then various implementations, but I'd like to avoid that
     * and trim the fat whenever possible
     *
     * I'll add more design notes here if they come but it should be simple enough :)
     */
    class IndexedTriangleMesh : public Renderable {
    public:
        struct CreationInput : public Renderable::CreationInput {
        };

        explicit IndexedTriangleMesh(const CreationInput &creationInput)
                : Renderable(creationInput) {

        }

    private:

    };

} // scene
