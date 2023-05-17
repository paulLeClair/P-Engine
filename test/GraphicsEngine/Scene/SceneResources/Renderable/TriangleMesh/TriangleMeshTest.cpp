
#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"

using namespace PGraphics;

class TriangleMeshTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::unique_ptr<TriangleMesh> mesh;
};

TEST_F(TriangleMeshTest, BasicCreation) {
    ASSERT_NO_THROW(mesh = std::make_unique<TriangleMesh>(TriangleMesh::CreationInput{
            scene,
            "test",
            UniqueIdentifier(),
            Renderable::RenderableType::TriangleMesh,
            {}
    }));
}