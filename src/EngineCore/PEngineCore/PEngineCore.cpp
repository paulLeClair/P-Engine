//
// Created by paull on 2022-08-05.
//

#include "PEngineCore.hpp"

namespace PEngine {
    EngineCore::RunResult PEngineCore::run() {
        if (engineModes.empty()) {
            return EngineCore::RunResult::FAILURE;
        }

        isEngineCoreRunning = true;

        try {
            auto initialEngineMode = engineModes[initialEngineModeName];
            if (initialEngineMode->run() != EngineMode::RunResult::SUCCESS) {
                return EngineCore::RunResult::FAILURE;
            }
        }
        catch (std::exception &exception) {
            isEngineCoreRunning = false;
            return EngineCore::RunResult::FAILURE;
        }

        return EngineCore::RunResult::SUCCESS;
    }

    bool PEngineCore::isRunning() {
        return isEngineCoreRunning;
    }
}// namespace PEngine