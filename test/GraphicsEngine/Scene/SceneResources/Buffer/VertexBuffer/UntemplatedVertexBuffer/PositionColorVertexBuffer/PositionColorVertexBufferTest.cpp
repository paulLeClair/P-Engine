#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorVertexBuffer/PositionColorVertexBuffer.hpp"
#include "../../../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"

using namespace PGraphics;

class PositionColorVertexBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });

        buffer = std::make_shared<PositionColorVertexBuffer>(PositionColorVertexBuffer::CreationInput{
                scene,
                "test",
                UniqueIdentifier(),
                [&](const Buffer &) {
                    // empty update callback
                }
        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<PositionColorVertexBuffer> buffer;
};

TEST_F(PositionColorVertexBufferTest, BasicCreation) {
    ASSERT_TRUE(buffer->isVertexBuffer());
    ASSERT_TRUE(buffer->isPositionColorVertexBuffer());
}