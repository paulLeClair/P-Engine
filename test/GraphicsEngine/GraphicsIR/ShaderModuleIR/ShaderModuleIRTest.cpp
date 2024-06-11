//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp"

using namespace pEngine::girEngine::gir;

class ShaderModuleIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<ShaderModuleIR> shaderModule = nullptr;
};

// note: shader module IR currently doesn't hold onto the actual shader binary itself

TEST_F(ShaderModuleIRTest, BasicCreationOfEmptyShaderModuleGir) {
    ASSERT_NO_THROW(shaderModule = std::make_shared<ShaderModuleIR>(ShaderModuleIR::CreationInput{
            "emptyShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "", // no filename
            ShaderModuleIR::IntermediateRepresentation::UNKNOWN,
            ShaderModuleIR::ShaderUsage::UNKNOWN
    }));
}

TEST_F(ShaderModuleIRTest, BasicCreationOfVertexShader) {
    ASSERT_NO_THROW(shaderModule = std::make_shared<ShaderModuleIR>(ShaderModuleIR::CreationInput{
            "emptyVertexShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "", // no filename
            ShaderModuleIR::IntermediateRepresentation::UNKNOWN,
            ShaderModuleIR::ShaderUsage::VERTEX_SHADER
    }));
}

TEST_F(ShaderModuleIRTest, BasicCreationOfFragmentShader) {
    ASSERT_NO_THROW(shaderModule = std::make_shared<ShaderModuleIR>(ShaderModuleIR::CreationInput{
            "emptyFragShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "", // no filename
            ShaderModuleIR::IntermediateRepresentation::UNKNOWN,
            ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER
    }));
}

// TODO - other shader module types