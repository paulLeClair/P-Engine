#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Sampler/Sampler.hpp"
#include "../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"

using namespace PGraphics;

class SamplerTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene = nullptr;

    std::shared_ptr<Sampler> testSampler = nullptr;
};

TEST_F(SamplerTest, BasicCreation) {
    ASSERT_NO_THROW(testSampler = std::make_shared<Sampler>(Sampler::CreationInput{
            scene,
            "test",
            UniqueIdentifier(),
            0.5,
            0.5,
            Sampler::MagnificationFilterType::NEAREST_TEXEL,
            Sampler::MinificationFilterType::NEAREST_TEXEL,
            Sampler::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER,
            Sampler::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT,
            Sampler::PercentageCloserFilteringCompareOperation::LESS_THAN_EQUALS,
            true,
            0,
            1,
            false,
            false
    }));
}

// TODO - expand these as the design of surrounding components continues