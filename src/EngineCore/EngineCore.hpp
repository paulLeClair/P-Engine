//
// Created by paull on 2022-05-30.
//

#pragma once

#include <memory>

#include "../GraphicsEngine/GraphicsEngine.hpp"
#include "ThreadPool/ThreadPool.hpp"

namespace PEngine {

    class EngineCore {
    public:
        ~EngineCore() = default;

        /**
         * Not sure if I'll keep this - but it should be a global indicator of what happened during execution -
         * we'll see if I keep it long term
         */
        enum class RunResult {
            SUCCESS,
            FAILURE
        };

        virtual RunResult run() = 0;

        virtual std::shared_ptr<ThreadPool> getThreadPool() = 0;

        virtual std::shared_ptr<GraphicsEngine> getGraphicsEngine() = 0;

        virtual bool isRunning() = 0;
    };

}// namespace PEngine