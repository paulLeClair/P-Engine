//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"
#include <glm/ext/matrix_float4x4.hpp>

using namespace pEngine::girEngine::gir;

class ShaderConstantIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<ShaderConstantIR> shaderConstant = nullptr;
};

TEST_F(ShaderConstantIRTest, BasicCreationWithNoData) {
    ASSERT_NO_THROW(shaderConstant = std::make_shared<ShaderConstantIR>(ShaderConstantIR::CreationInput{
            "testShaderConstant",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_CONSTANT,
            nullptr,
            0
    }));
}

TEST_F(ShaderConstantIRTest, BasicCreationWithData) {
    static const glm::mat4 shaderConstantTestMatrixData = glm::mat4(
            glm::vec4(0.0f),
            glm::vec4(1.0f),
            glm::vec4(2.0f),
            glm::vec4(3.0f));

    std::string testName = "testName";
    pEngine::util::UniqueIdentifier testUid = pEngine::util::UniqueIdentifier();
    ASSERT_NO_THROW(shaderConstant = std::make_shared<ShaderConstantIR>(ShaderConstantIR::CreationInput{
            testName,
            testUid,
            GIRSubtype::SHADER_CONSTANT,
            reinterpret_cast<const unsigned char *>(&shaderConstantTestMatrixData),
            sizeof(glm::mat4)
    }));

    ASSERT_EQ(testName, shaderConstant->getName());
    ASSERT_EQ(testUid, shaderConstant->getUid());
    ASSERT_EQ(shaderConstantTestMatrixData, *(shaderConstant->getRawDataContainer()->getRawDataPointer<glm::mat4>()));
}