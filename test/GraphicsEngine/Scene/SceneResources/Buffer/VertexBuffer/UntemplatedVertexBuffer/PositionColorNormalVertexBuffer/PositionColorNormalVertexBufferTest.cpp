#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalVertexBuffer/PositionColorNormalVertexBuffer.hpp"
#include "../../../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"

using namespace PGraphics;

class PositionColorNormalVertexBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{});

        buffer = std::make_shared<PositionColorNormalVertexBuffer>(PositionColorNormalVertexBuffer::CreationInput{
                scene,
                "test",
                PUtilities::UniqueIdentifier(),
                [&](const Buffer &buf) {
                    //
                }
        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<PositionColorNormalVertexBuffer> buffer;
};

TEST_F(PositionColorNormalVertexBufferTest, BasicCreation) {
    ASSERT_TRUE(buffer->isPositionColorNormalVertexBuffer());
}