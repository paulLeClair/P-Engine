//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp"
#include "../../../../../src/lib/glm/vec3.hpp"

using namespace pEngine::girEngine::gir;

class ImageIRTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<ImageIR> image = nullptr;
};

TEST_F(ImageIRTest, BasicCreationWithNoData) {
    ASSERT_NO_THROW(
        image = std::make_shared<ImageIR>(ImageIR::CreationInput{
            "testImage",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::IMAGE,
            {ImageIR::ImageUsage::COLOR_ATTACHMENT},
            nullptr,
            0
            })
    );
}


TEST_F(ImageIRTest, BasicCreationWithData) {
    using TestVectorType = glm::vec3;

    static const std::vector<TestVectorType> UNREALISTIC_TEST_DATA = {
        glm::vec3(0.0),
        glm::vec3(1.0),
        glm::vec3(2.0)
    };

    const std::string testName = "testImage";
    const auto testUid = pEngine::util::UniqueIdentifier();
    constexpr ImageIR::ImageUsage testImageUsages = ImageIR::ImageUsage::COLOR_ATTACHMENT;

    ASSERT_NO_THROW(
        image = std::make_shared<ImageIR>(ImageIR::CreationInput{
            testName,
            testUid,
            GIRSubtype::IMAGE,
            testImageUsages,
            reinterpret_cast<const unsigned char *>(UNREALISTIC_TEST_DATA.data()),
            static_cast<unsigned long>(UNREALISTIC_TEST_DATA.size() * sizeof(TestVectorType))
            })
    );
    ASSERT_EQ(testName, image->getName());
    ASSERT_EQ(testUid, image->getUid());
    ASSERT_EQ(testImageUsages, image->getImageUsage());
    ASSERT_EQ(UNREALISTIC_TEST_DATA, image->getImageData().getRawDataAsVector<TestVectorType>());
}
