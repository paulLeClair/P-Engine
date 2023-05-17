
#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.hpp"

using namespace PGraphics;

class IndexedTriangleMeshTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::unique_ptr<IndexedTriangleMesh> mesh;
};

TEST_F(IndexedTriangleMeshTest, BasicCreation) {
    ASSERT_NO_THROW(mesh = std::make_unique<IndexedTriangleMesh>(IndexedTriangleMesh::CreationInput{
            scene,
            "test",
            UniqueIdentifier(),
            Renderable::RenderableType::IndexedTriangleMesh,
            {},
            {}
    }));
}