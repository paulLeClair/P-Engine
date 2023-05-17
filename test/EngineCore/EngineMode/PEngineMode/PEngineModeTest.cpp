//
// Created by paull on 2023-04-04.
//

#include <gtest/gtest.h>
#include "../../../../src/EngineCore/EngineMode/PEngineMode/PEngineMode.hpp"
#include "../../../../src/EngineCore/PEngineCore/PEngineCore.hpp"

using namespace PEngine;

class PEngineModeTest : public ::testing::Test {
protected:
    void SetUp() override {
        engineCore = std::make_shared<PEngineCore>(PEngineCore::CreationInput{
                std::thread::hardware_concurrency(),
                {},
                ""
        });

        engineMode = std::make_shared<PEngineMode>(PEngineMode::CreationInput{
                "test base engine mode",
                engineCore
        });
    }

    std::shared_ptr<PEngineMode> uninitializedEngineMode = nullptr;

    std::shared_ptr<PEngineMode> engineMode = nullptr;

    std::shared_ptr<PEngineCore> engineCore = nullptr;
};

TEST_F(PEngineModeTest, BasicCreation) {
    ASSERT_NO_THROW(uninitializedEngineMode = std::make_shared<PEngineMode>(PEngineMode::CreationInput{
            "test base engine mode",
            engineCore
    }));
}

TEST_F(PEngineModeTest, EnsureRunFailsOnBasePEngineMode) {
    ASSERT_TRUE(engineMode->run() == EngineMode::RunResult::FAILURE);
}