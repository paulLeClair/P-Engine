#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Image/Image.hpp"

using namespace pEngine::girEngine::scene;

class ImageTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

};

TEST_F(ImageTest, BasicCreation) {
    ASSERT_NO_THROW(std::shared_ptr<Image> testImage = std::make_shared<Image>(Image::CreationInput{
            "testImage",
            pEngine::util::UniqueIdentifier(),
            TexelFormat::R8G8B8A8_SRGB,
            Image::ImageExtent2D{
                    100,
                    100
            },
            Image::MipMapConfiguration{
                    false,
                    1,
                    false
            },
            Image::ImageArrayConfiguration{
                    false,
                    1
            },
            Image::TexelTilingArrangement::BACKEND_OPTIMAL,
            1,
            {Image::ImageUsage::ColorAttachment},
            nullptr,
            0
    }));
}