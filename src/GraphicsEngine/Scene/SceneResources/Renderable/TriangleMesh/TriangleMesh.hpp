//
// Created by paull on 2022-12-14.
//

#pragma once

#include <memory>
#include "../Renderable.hpp"
#include "../../../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../Scene.hpp"
#include "../../Buffer/VertexBuffer/VertexBuffer.hpp"


class TriangleMesh : public Renderable {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        RenderableType renderableType;

        std::vector<std::shared_ptr<Buffer>> initialVertexBuffers;
    };

    explicit TriangleMesh(const CreationInput &creationInput);

    UpdateResult update() override;

private:
    std::vector<std::shared_ptr<Buffer>> vertexBuffers;
};
