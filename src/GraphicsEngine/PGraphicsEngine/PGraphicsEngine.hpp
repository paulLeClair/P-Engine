//
// Created by paull on 2022-08-26.
//

#pragma once

#include <memory>
#include <utility>

#include "../Backend/PBackend/PBackend.hpp"
#include "../GraphicsEngine.hpp"
#include "../Scene/Scene.hpp"

namespace PGraphics {

    class PGraphicsEngine : public GraphicsEngine {
    public:
        struct CreationInput {
            std::shared_ptr<PEngine::PThreadPool> threadPool;
        };

        explicit PGraphicsEngine(const CreationInput &creationInput) : threadPool(creationInput.threadPool) {
            scene = std::make_shared<PScene>(PScene::CreationInput{});

            PBackend::CreationInput backendCreateInfo = {std::move(threadPool), scene};

            backend = std::make_shared<PBackend>(backendCreateInfo);
        }

        ~PGraphicsEngine() = default;

        std::shared_ptr<Scene> getScene() override {
            return scene;
        }

        std::shared_ptr<Backend> getBackend() override {
            return backend;
        }

        BakeResult bakeRenderData() override;

        DrawFrameResult drawNextFrame() override;

    private:

        std::shared_ptr<PEngine::PThreadPool> threadPool = nullptr;

        std::shared_ptr<PScene> scene = nullptr;

        std::shared_ptr<PBackend> backend = nullptr;

    protected:
        friend void PBackend::bakeRenderData();
    };

}// namespace PGraphics
