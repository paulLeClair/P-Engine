//
// Created by paull on 2022-12-14.
//

#pragma once

#include <memory>
#include "../Renderable.hpp"
#include "../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../Scene.hpp"
#include "../../Buffer/VertexBuffer/VertexBuffer.hpp"

namespace pEngine::girEngine::scene {

    class TriangleMesh : public Renderable {
    public:
        struct CreationInput : public Renderable::CreationInput {
        };

        explicit TriangleMesh(const CreationInput &creationInput)
                : Renderable(creationInput) {

        }

    };

}