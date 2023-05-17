//
// Created by paull on 2022-08-05.
//

#pragma once

#include <memory>

#include "../../GraphicsEngine/PGraphicsEngine/PGraphicsEngine.hpp"
#include "../EngineCore.hpp"
#include "../EngineMode/PEngineMode/PEngineMode.hpp"
#include "../ThreadPool/PThreadPool/PThreadPool.hpp"

namespace PEngine {

    class PEngineCore : public EngineCore {
    public:
        struct CreationInput {
            uint32_t numberOfThreadPoolThreads = std::thread::hardware_concurrency();

            std::unordered_map<std::string, std::shared_ptr<PEngineMode::CreationInput>> engineModeCreateInfos = {};

            std::string initialEngineMode;
        };

        explicit PEngineCore(const CreationInput &createInfo) {
            PThreadPool::CreationInput threadPoolCreateInfo{createInfo.numberOfThreadPoolThreads};
            threadPool = std::make_shared<PThreadPool>(threadPoolCreateInfo);

            graphicsEngine = std::make_shared<PGraphics::PGraphicsEngine>(
                    PGraphics::PGraphicsEngine::CreationInput{threadPool});

            for (auto &engineModeNameAndCreateInfo: createInfo.engineModeCreateInfos) {
                engineModes[engineModeNameAndCreateInfo.first] = std::make_shared<PEngine::PEngineMode>(
                        *engineModeNameAndCreateInfo.second);
            }
        }

        ~PEngineCore() = default;

        // TODO - determine if this kind of approach would work well elsewhere? (and here?)
        std::shared_ptr<ThreadPool> getThreadPool() override {
            return threadPool;
        };

        RunResult run() override;

        std::shared_ptr<GraphicsEngine> getGraphicsEngine() override {
            return graphicsEngine;
        }

        bool isRunning() override;

    private:
        bool isEngineCoreRunning = false;

        std::shared_ptr<PThreadPool> threadPool;

        std::shared_ptr<PGraphics::PGraphicsEngine> graphicsEngine;

        std::unordered_map<std::string, std::shared_ptr<PEngine::PEngineMode>> engineModes;

        std::string initialEngineModeName;
    };

}// namespace PEngine
