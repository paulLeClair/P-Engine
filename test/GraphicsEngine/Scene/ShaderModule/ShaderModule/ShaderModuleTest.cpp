//
// Created by paull on 2022-06-19.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/ShaderModule/ShaderModule.hpp"

using namespace pEngine::girEngine::scene;

static const char *const TEST_VERTEX_SHADER_SPV_FILENAME = "testVertShader";

static const char *const TEST_FRAGMENT_SHADER_SPV_FILENAME = "testFragShader";

class ShaderModuleTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<ShaderModule> shaderModule = nullptr;

};

TEST_F(ShaderModuleTest, CreateBasicVertexShaderModule) {
    shaderModule = std::make_shared<ShaderModule>(ShaderModule::CreationInput{
            "testVertexShaderModule",
            pEngine::util::UniqueIdentifier(),
            TEST_VERTEX_SHADER_SPV_FILENAME,
            "main",
            ShaderModule::ShaderUsage::VERTEX_SHADER,
            ShaderModule::ShaderLanguage::GLSL
    });
}

TEST_F(ShaderModuleTest, CreateBasicFragmentShaderModule) {
    shaderModule = std::make_shared<ShaderModule>(ShaderModule::CreationInput{
            "testFragmentShaderModule",
            pEngine::util::UniqueIdentifier(),
            TEST_FRAGMENT_SHADER_SPV_FILENAME,
            "main",
            ShaderModule::ShaderUsage::FRAGMENT_SHADER,
            ShaderModule::ShaderLanguage::GLSL
    });
}

TEST_F(ShaderModuleTest, CreateBasicGeometryShaderModule) {
    // TODO
}

TEST_F(ShaderModuleTest, CreateBasicTessControlShaderModule) {
    // TODO
}

TEST_F(ShaderModuleTest, CreateBasicTessEvalShaderModule) {
    // TODO
}

TEST_F(ShaderModuleTest, CreateBasicMeshShaderModule) {
    // TODO
}