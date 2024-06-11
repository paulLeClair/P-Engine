#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.hpp"

using namespace pEngine::girEngine::scene;

class IndexedTriangleMeshTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::unique_ptr<IndexedTriangleMesh> mesh;
};

TEST_F(IndexedTriangleMeshTest, BasicCreation) {
    ASSERT_NO_THROW(mesh = std::make_unique<IndexedTriangleMesh>(IndexedTriangleMesh::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            Renderable::RenderableType::INDEXED_TRIANGLE_MESH,
            {},
            {}
    }));
}