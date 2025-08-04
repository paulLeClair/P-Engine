#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Image/Image.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Texture/Texture.hpp"

using namespace pEngine::girEngine::scene;

class TextureTest : public ::testing::Test {
protected:
    void SetUp() override {
        sampler = SamplerSettings{
                0.0,
                0.0,
                SamplerSettings::MagnificationFilterType::NEAREST_TEXEL,
                SamplerSettings::MinificationFilterType::NEAREST_TEXEL,
                SamplerSettings::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER,
                SamplerSettings::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT,
                false,
                false,
                false,
                0,
                0,
                SamplerSettings::PercentageCloserFilteringCompareOperation::LESS_THAN,
        };

        image = std::make_shared<Image>(Image::CreationInput{
                "test",
                UniqueIdentifier(),
                ResourceFormat::R8G8B8A8_SRGB,
                Image::ImageExtent2D{0, 0},
                Image::MipMapConfiguration{
                        false,
                        0,
                        false
                },
                Image::ImageArrayConfiguration{
                        false,
                        1
                },
                Image::TexelTilingArrangement::BACKEND_OPTIMAL,
                1,
                {Image::ImageUsage::SampledTextureImage},
                nullptr,
                0
        });
    }

    SamplerSettings sampler;

    std::shared_ptr<Image> image;

    std::unique_ptr<Texture> texture;
};

TEST_F(TextureTest, BasicCreation) {
    ASSERT_NO_THROW(texture = std::make_unique<Texture>(Texture::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            sampler,
            image
    }));
}