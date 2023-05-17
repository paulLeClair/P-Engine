#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBuffer.hpp"

using namespace PGraphics;

class TexelBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<Buffer> buffer;
};

TEST_F(TexelBufferTest, BasicCreation) {
    std::shared_ptr<Sampler> texelBufferSampler = std::make_shared<Sampler>(Sampler::CreationInput{

    });

    ASSERT_NO_THROW(buffer = std::make_shared<TexelBuffer>(TexelBuffer::CreationInput{
            scene,
            "test texel buffer",
            PUtilities::UniqueIdentifier(),
            [&](const Buffer &buffer) {
                // nothing
            },
            TexelBuffer::UsageType::UNIFORM,
            nullptr,
            0,
            texelBufferSampler
    }));

}
