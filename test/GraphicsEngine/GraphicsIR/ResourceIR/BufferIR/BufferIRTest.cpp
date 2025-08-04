//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp"
#include <glm/vec3.hpp>

using namespace pEngine::girEngine::gir;

class BufferIRTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<BufferIR> buffer = nullptr;
};

TEST_F(BufferIRTest, BasicCreationWithNoData) {
    ASSERT_NO_THROW(buffer = std::make_shared<BufferIR>(BufferIR::CreationInput{
            "testBufferIR",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::BUFFER,
            {BufferIR::BufferUsage::UNIFORM_BUFFER}, // arbitrary buffer usage
            nullptr,
            0
    }));
}

TEST_F(BufferIRTest, BasicCreationWithData) {
    using TestVectorType = glm::vec<3, float, glm::qualifier::highp>;

    static const TestVectorType &firstVector = glm::vec3(0.0);
    static const TestVectorType &secondVector = glm::vec3(1.0);
    static const TestVectorType &thirdVector = glm::vec3(2.0);

    static const std::vector<TestVectorType> TEST_DATA = {
            firstVector,
            secondVector,
            thirdVector
    };

    const char *testName = "testBufferIR";
    const pEngine::util::UniqueIdentifier &testUid = pEngine::util::UniqueIdentifier();
    constexpr auto testBufferUsage = BufferIR::BufferUsage::UNIFORM_BUFFER;

    ASSERT_NO_THROW(buffer = std::make_shared<BufferIR>(BufferIR::CreationInput{
            testName,
            testUid,
            GIRSubtype::BUFFER,
            {testBufferUsage},
            reinterpret_cast<const unsigned char *>(TEST_DATA.data()),
            static_cast<unsigned long>(TEST_DATA.size() * sizeof(TestVectorType))
    }));

    constexpr auto testUsages = testBufferUsage;
    ASSERT_EQ(testName, buffer->getName());
    ASSERT_EQ(testUid, buffer->getUid());
    ASSERT_EQ(testUsages, buffer->getUsage());
    ASSERT_EQ(TEST_DATA, buffer->getRawDataContainer().getRawDataAsVector<TestVectorType>());
}

// TODO - add more tests if needed (GIR classes are generally just meant to organize & store the data though)
