//
// Created by paull on 2022-08-05.
//

#include <gtest/gtest.h>
#include "../../../src/EngineCore/PEngineCore/PEngineCore.hpp"

using namespace PEngine;

class PEngineCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        core = std::make_shared<PEngineCore>(
                PEngineCore::CreationInput{
                        std::thread::hardware_concurrency(),
                        {},
                        ""
                });

    }

    std::shared_ptr<PEngine::PEngineCore> core = nullptr;
};

TEST_F(PEngineCoreTest, BasicEngineCoreCreationWithNoEngineModes) {
    ASSERT_TRUE(core->run() == EngineCore::RunResult::FAILURE);
    ASSERT_TRUE(!core->isRunning());
}