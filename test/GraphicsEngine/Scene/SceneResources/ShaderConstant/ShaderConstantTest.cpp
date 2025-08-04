#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstant.hpp"

using namespace pEngine::girEngine::scene;

class ShaderConstantTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::unique_ptr<ShaderConstant> shaderConstant;
};

TEST_F(ShaderConstantTest, BasicCreation) {
    ASSERT_NO_THROW(shaderConstant = std::make_unique<ShaderConstant>(ShaderConstant::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            {},
            0,
            0,
            nullptr,
            0
    }));
}