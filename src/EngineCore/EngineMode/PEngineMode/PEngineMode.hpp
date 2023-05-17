//
// Created by paull on 2022-08-30.
//

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "../EngineMode.hpp"

namespace PEngine {

    class PEngineCore;

    class PEngineMode : public EngineMode {
    public:
        struct CreationInput {
            std::string name;

            std::shared_ptr<PEngineCore> engineCore = nullptr;

            CreationInput(std::string name, std::shared_ptr<PEngineCore> engineCore) : name(std::move(name)),
                                                                                       engineCore(
                                                                                               std::move(engineCore)) {

            }
        };

        explicit PEngineMode(const CreationInput &createInfo);

        ~PEngineMode() = default;

        RunResult run() override;

    protected:
        std::string name;

        std::shared_ptr<PEngineCore> engineCore;

        enum class PrepareFrameResult {
            SUCCESS,
            STOP_RUNNING,
            FAILURE
        };

        virtual PrepareFrameResult prepareFrame() {
            return PrepareFrameResult::FAILURE;
        }

        enum class DrawFrameResult {
            SUCCESS,
            FAILURE
        };

        virtual DrawFrameResult drawFrame() {
            return DrawFrameResult::FAILURE;
        }
    };

}// namespace PEngine
