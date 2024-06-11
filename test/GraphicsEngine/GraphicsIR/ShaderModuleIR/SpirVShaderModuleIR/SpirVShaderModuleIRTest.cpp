//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/GraphicsIR/ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIR.hpp"

using namespace pEngine::girEngine::gir;

class SpirVShaderModuleIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    const std::vector<unsigned int> TEST_SPIR_V_CODE = {1, 2, 3}; // junk spirv byte code

    std::shared_ptr<SpirVShaderModuleIR> shaderModule = nullptr;
};

TEST_F(SpirVShaderModuleIRTest, BasicCreationOfEmptyShaderModuleGir) {
    ASSERT_NO_THROW(shaderModule = std::make_shared<SpirVShaderModuleIR>(SpirVShaderModuleIR::CreationInput{
            "emptyShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "",
            ShaderModuleIR::IntermediateRepresentation::SPIR_V,
            ShaderModuleIR::ShaderUsage::UNKNOWN,
            {} // no shader data
    }));
}

TEST_F(SpirVShaderModuleIRTest, BasicCreationOfVertexShader) {


    ASSERT_NO_THROW(shaderModule = std::make_shared<SpirVShaderModuleIR>(SpirVShaderModuleIR::CreationInput{
            "vertexShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "",
            ShaderModuleIR::IntermediateRepresentation::SPIR_V,
            ShaderModuleIR::ShaderUsage::VERTEX_SHADER,
            "",
            TEST_SPIR_V_CODE
    }));
}

TEST_F(SpirVShaderModuleIRTest, BasicCreationOfFragmentShader) {
    ASSERT_NO_THROW(shaderModule = std::make_shared<SpirVShaderModuleIR>(SpirVShaderModuleIR::CreationInput{
            "fragShaderModuleGir",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::SHADER_MODULE,
            "",
            ShaderModuleIR::IntermediateRepresentation::SPIR_V,
            ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER,
            "",
            TEST_SPIR_V_CODE
    }));
}

// TODO - other shader module types