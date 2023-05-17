//
// Created by paull on 2022-09-02.
//

#pragma once

#include <memory>
#include <utility>

#include "../../../PEngineCore/PEngineCore.hpp"
#include "../PEngineMode.hpp"

namespace PGraphics {
    class PImguiRenderPass;
}

namespace PEngine {

    class CoreMenuEngineMode : public PEngineMode {
    public:
        struct CreationInput : public PEngineMode::CreationInput {
            CreationInput(const std::string &name, std::shared_ptr<PEngine::PEngineCore> core)
                    : PEngineMode::CreationInput(name, std::move(core)) {

            }
        };

        explicit CoreMenuEngineMode(const CreationInput &creationInput);

        ~CoreMenuEngineMode() = default;

        RunResult run() override;

    private:
        static const std::string DEFAULT_IMGUI_RENDER_PASS_NAME;

        std::shared_ptr<ApplicationContext> applicationContext;

        std::shared_ptr<FrameExecutionController> frameContext;

        std::shared_ptr<PGraphics::PImguiRenderPass> imguiRenderPass;

        PrepareFrameResult prepareFrame() override;

        DrawFrameResult drawFrame() override;

        void showApplicationWindow();

        void createImguiRenderPass();

        static std::vector<std::function<void()>> getInitialGuiElementCallbacks();

        GraphicsEngine::BakeResult configureSceneAndBakeRenderData();
    };

}// namespace PEngine
