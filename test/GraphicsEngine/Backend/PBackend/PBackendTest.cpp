//
// Created by paull on 2022-06-19.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/Backend/PBackend/PBackend.hpp"

using namespace PGraphics;
using namespace PEngine;

class PBackendTest : public ::testing::Test {
protected:
    void SetUp() override {
        threadPool = std::make_shared<PThreadPool>(PThreadPool::CreationInput{
                std::thread::hardware_concurrency()
        });

        scene = std::make_shared<PScene>(PScene::CreationInput{

        });

        testBackend = std::make_shared<PBackend>(PBackend::CreationInput{
                threadPool,
                scene
        });
    }

    std::shared_ptr<PThreadPool> threadPool;

    std::shared_ptr<PScene> scene;

    std::shared_ptr<PBackend> testBackend;

};

TEST_F(PBackendTest, BasicCreation) {

}

// TODO - extend these tests as PBackend becomes more configurable/complicated