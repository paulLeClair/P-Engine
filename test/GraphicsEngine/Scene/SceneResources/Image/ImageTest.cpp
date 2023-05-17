#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Image/Image.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/TexelFormat/R8G8B8A8_SRGB_TexelFormat/R8G8B8A8_SRGB_TexelFormat.hpp"

using namespace PGraphics;

class ImageTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<PScene> scene;

};

TEST_F(ImageTest, BasicCreation) {
    ASSERT_NO_THROW(std::shared_ptr<Image> testImage = std::make_shared<Image>(Image::CreationInput{
            scene,
            "testImage",
            PUtilities::UniqueIdentifier(),
            std::make_shared<R8G8B8A8_SRGB_TexelFormat>(),
            Image::ImageExtent2D(0, 0),
            1,
            std::unordered_set<Image::ImageUsage>(),
            nullptr,
            0
    }));
}