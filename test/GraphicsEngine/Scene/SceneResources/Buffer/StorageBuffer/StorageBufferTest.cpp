#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBuffer.hpp"

using namespace PGraphics;

class StorageBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<Buffer> buffer;
};

TEST_F(StorageBufferTest, BasicCreation) {
    ASSERT_NO_THROW(buffer = std::make_shared<StorageBuffer>(StorageBuffer::CreationInput{
            scene,
            "testStorageBuffer",
            UniqueIdentifier(),
            [&](const Buffer &) {
                // nothing
            }
    }));

}