
#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Backend/FrameExecutionController/Frame/PFrame/VulkanFrame.hpp"

using namespace PGraphics;
using namespace PEngine;

class PFrameTest : public ::testing::Test {
protected:
    void SetUp() override {
        applicationContext = std::make_shared<PApplicationContext>();

        // TODO - fix this when we get render passes implemented fully
//        frame = std::make_shared<VulkanFrame>(VulkanFrame::CreationInput{
//                0,
//                applicationContext->getLogicalDevice(),
//                threadPool,
//                {},
//                nullptr,
//                applicationContext->getGraphicsQueueFamilyIndex(),
//                VK_NULL_HANDLE,
//                applicationContext->getGraphicsQueue()
//        });
    }

    std::shared_ptr<PApplicationContext> applicationContext;

    std::shared_ptr<PThreadPool> threadPool;

    std::shared_ptr<PFrame> frame;

};

TEST_F(PFrameTest, BasicFrameCreation) {

}

// TODO - flesh out these tests as the frame execution logic is put in place