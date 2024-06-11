//
// Created by paull on 2022-06-19.
//

#include <gtest/gtest.h>

// TODO - gut and rework this whole test file

class PFrameExecutionControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        //        applicationContext = std::make_shared<VulkanApplicationContext>();

        //        scene = std::make_shared<PScene>(PScene::CreationInput{
        //
        //        });

        //        threadPool = std::make_shared<PThreadPool>(PThreadPool::CreationInput{
        //                std::thread::hardware_concurrency()
        //        });

        //        frameExecutionController = std::make_shared<VulkanFrameExecutionController>(VulkanFrameExecutionController::CreationInput{
        //                applicationContext,
        //                scene,
        //                threadPool,
        //                3
        //        });
    }

    //
    //    std::shared_ptr<VulkanApplicationContext> applicationContext;
    //
    //    std::shared_ptr<PScene> scene;
    //
    //    std::shared_ptr<PThreadPool> threadPool;
    //
    //    std::shared_ptr<VulkanFrameExecutionController> frameExecutionController;
};

TEST_F(PFrameExecutionControllerTest, Create) {
    //    ASSERT_TRUE(frameExecutionController.get() != nullptr);
}

TEST_F(PFrameExecutionControllerTest, BakeEmptyScene) {
    // TODO
}

TEST_F(PFrameExecutionControllerTest, BakeEmptySceneAndDrawFrame) {
    // TODO
}

// TODO - expand these tests once the bake code is ready
