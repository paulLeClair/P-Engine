#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/TexelFormat/R8G8B8A8_SRGB_TexelFormat/R8G8B8A8_SRGB_TexelFormat.hpp"

using namespace PGraphics;

class TextureTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });

        sampler = std::make_shared<Sampler>(Sampler::CreationInput{
                scene,
                "test",
                UniqueIdentifier(),
                0.0,
                0.0,
                Sampler::MagnificationFilterType::NEAREST_TEXEL,
                Sampler::MinificationFilterType::NEAREST_TEXEL,
                Sampler::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER,
                Sampler::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT,
                Sampler::PercentageCloserFilteringCompareOperation::LESS_THAN,
                false,
                0,
                0,
                false,
                false
        });

        image = std::make_shared<Image>(Image::CreationInput{
                scene,
                "test",
                UniqueIdentifier(),
                std::make_shared<R8G8B8A8_SRGB_TexelFormat>(),
                Image::ImageExtent2D(0, 0),
                1,
                {},
                nullptr,
                0
        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<Sampler> sampler;

    std::shared_ptr<Image> image;

    std::unique_ptr<Texture> texture;
};

TEST_F(TextureTest, BasicCreation) {
    ASSERT_NO_THROW(texture = std::make_unique<Texture>(Texture::CreationInput{
            scene,
            "test",
            UniqueIdentifier(),
            Texture::ResourceConfiguration::SAMPLER_ONLY,
            sampler,
            image
    }));
}