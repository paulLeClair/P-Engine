//
// Created by paull on 2022-08-27.
//

#include <gtest/gtest.h>
#include "../../../src/GraphicsEngine/PGraphicsEngine/PGraphicsEngine.hpp"

using namespace PEngine;
using namespace PGraphics;

class PGraphicsEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        threadPool = std::make_shared<PThreadPool>(PThreadPool::CreationInput{
                std::thread::hardware_concurrency()
        });
    }

    std::shared_ptr<PThreadPool> threadPool = nullptr;

    std::shared_ptr<PGraphicsEngine> graphicsEngine = nullptr;

};

TEST_F(PGraphicsEngineTest, CreateGraphicsEngine) {
    ASSERT_NO_THROW(graphicsEngine = std::make_shared<PGraphicsEngine>(PGraphicsEngine::CreationInput{
            threadPool
    }));
    ASSERT_NE(graphicsEngine->getScene(), nullptr);
    ASSERT_NE(graphicsEngine->getBackend(), nullptr);
}

// TODO - extend these tests as the designs of sub-components begin to solidify

