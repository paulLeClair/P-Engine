
#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"

using namespace PGraphics;

class ShaderConstantTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::unique_ptr<ShaderConstant> shaderConstant;
};

TEST_F(ShaderConstantTest, BasicCreation) {
    ASSERT_NO_THROW(shaderConstant = std::make_unique<ShaderConstant>(ShaderConstant::CreationInput{
            scene,
            "test",
            UniqueIdentifier(),
            {},
            0,
            0,
            nullptr,
            0
    }));
}