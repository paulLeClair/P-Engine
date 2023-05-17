//
// Created by paull on 2022-06-19.
//

#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Backend/FrameExecutionController/PFrameExecutionController/PFrameExecutionController.hpp"

using namespace PGraphics;
using namespace PEngine;

class PFrameExecutionControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        applicationContext = std::make_shared<PApplicationContext>();

        scene = std::make_shared<PScene>(PScene::CreationInput{

        });

        threadPool = std::make_shared<PThreadPool>(PThreadPool::CreationInput{
                std::thread::hardware_concurrency()
        });

        frameExecutionController = std::make_shared<PFrameExecutionController>(PFrameExecutionController::CreationInput{
                applicationContext,
                scene,
                threadPool,
                3
        });
    }

    std::shared_ptr<PApplicationContext> applicationContext;

    std::shared_ptr<PScene> scene;

    std::shared_ptr<PThreadPool> threadPool;

    std::shared_ptr<PFrameExecutionController> frameExecutionController;
};

TEST_F(PFrameExecutionControllerTest, Create) {
    ASSERT_TRUE(frameExecutionController.get() != nullptr);
    // TODO - add getters and test that they return valid default values when that becomes more applicable
}

TEST_F(PFrameExecutionControllerTest, BakeEmptyScene) {
    // TODO
}

TEST_F(PFrameExecutionControllerTest, BakeEmptySceneAndDrawFrame) {
    // TODO
}

// TODO - expand these tests once the bake code is ready