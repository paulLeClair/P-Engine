//
// Created by paull on 2022-12-14.
//

#include "TriangleMesh.hpp"

TriangleMesh::TriangleMesh(const TriangleMesh::CreationInput &creationInput) : Renderable(Renderable::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.renderableType
}), vertexBuffers(creationInput.initialVertexBuffers) {

}

SceneResource::UpdateResult TriangleMesh::update() {
    // TODO
    return UpdateResult::SUCCESS;
}
