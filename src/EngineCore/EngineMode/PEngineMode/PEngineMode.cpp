//
// Created by paull on 2022-08-30.
//

#include "PEngineMode.hpp"

#include <stdexcept>

namespace PEngine {
    PEngineMode::PEngineMode(const PEngineMode::CreationInput &createInfo)
            : name(createInfo.name),
              engineCore(createInfo.engineCore) {
        if (createInfo.name.empty() || createInfo.engineCore == nullptr) {
            throw std::runtime_error("Invalid PEngineMode create info!");// TODO - factor this into a separate function
        }
    }

    EngineMode::RunResult PEngineMode::run() {
        return EngineMode::RunResult::FAILURE;
    }
}// namespace PEngine