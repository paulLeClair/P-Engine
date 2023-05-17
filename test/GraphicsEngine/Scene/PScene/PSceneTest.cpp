//
// Created by paull on 2022-06-17.
//

#include "../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../src/GraphicsEngine/Scene/SceneRenderGraph/PSceneRenderGraph/PSceneRenderGraph.hpp"

#include <gtest/gtest.h>

using namespace PGraphics;

class PSceneTest : public ::testing::Test {
protected:
    void SetUp() override {
        testScene = std::make_shared<PScene>(PScene::CreationInput{});
    }

    std::shared_ptr<PScene> testScene;
};

TEST_F(PSceneTest, CreateScene) {
    ASSERT_TRUE(testScene->getSceneRenderGraph() != nullptr);
    ASSERT_TRUE(testScene->getAllResources().empty());
    ASSERT_TRUE(testScene->getShaderModules().empty());
}

/* TODO - implement & extend these tests as the Scene become more complicated */

TEST_F(PSceneTest, AddImage) {
    // TODO
}

TEST_F(PSceneTest, AddBuffer) {
    // TODO
}

TEST_F(PSceneTest, AddShaderModule) {
    // TODO
}

TEST_F(PSceneTest, BindImageToSingleRenderPass) {
    // TODO
}

TEST_F(PSceneTest, BindBufferToSingleRenderPass) {
    // TODO
}

TEST_F(PSceneTest, BindShaderModuleToSingleRenderPass) {
    // TODO
}