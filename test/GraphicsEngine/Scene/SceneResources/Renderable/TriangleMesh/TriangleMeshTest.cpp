#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"

using namespace pEngine::girEngine::scene;

class TriangleMeshTest : public ::testing::Test {
protected:
    void SetUp() override {

    }


    std::unique_ptr<TriangleMesh> mesh;
};

TEST_F(TriangleMeshTest, BasicCreation) {
    ASSERT_NO_THROW(mesh = std::make_unique<TriangleMesh>(TriangleMesh::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            Renderable::RenderableType::TRIANGLE_MESH,
            {},
            {}
    }));
}