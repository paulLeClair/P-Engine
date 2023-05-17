//
// Created by paull on 2022-09-02.
//

#include "CoreMenuEngineMode.hpp"
#include "../../../../GraphicsEngine/Scene/SceneRenderGraph/ImguiRenderPass/PImguiRenderPass/PImguiRenderPass.hpp"

namespace PEngine {
    const std::string CoreMenuEngineMode::DEFAULT_IMGUI_RENDER_PASS_NAME("mainMenu");

    std::vector<std::function<void()>> CoreMenuEngineMode::getInitialGuiElementCallbacks() {
        // here i'll just plug in some initial stuff to see if it works...

        std::vector<std::function<void()>> initialGuiElementCallbacks = std::vector<std::function<void()>>();


        return initialGuiElementCallbacks;
    }

    CoreMenuEngineMode::CoreMenuEngineMode(const CoreMenuEngineMode::CreationInput &creationInput)
            : applicationContext(creationInput.engineCore->getGraphicsEngine()->getBackend()->getApplicationContext()),
              frameContext(creationInput.engineCore->getGraphicsEngine()->getBackend()->getFrameExecutionController()),
              PEngineMode(creationInput) {
        if (applicationContext == nullptr || frameContext == nullptr) {
            throw std::runtime_error(
                    "Unable to CoreMenuEngineMode: provided frame context or application context are null!");
        }
    }

    GraphicsEngine::BakeResult CoreMenuEngineMode::configureSceneAndBakeRenderData() {
        const std::shared_ptr<GraphicsEngine> &graphicsEngine = engineCore->getGraphicsEngine();

        // acquire scene and render graph handles for configuration
        const auto &sceneManager = graphicsEngine->getScene();
        const auto &renderGraph = sceneManager->getSceneRenderGraph();

        createImguiRenderPass();

        // BAKE the scene & render graph to end up with backend-specific rendering code
        auto bakeResult = graphicsEngine->bakeRenderData();
        return bakeResult;
    }

    void CoreMenuEngineMode::createImguiRenderPass() {
        PGraphics::PImguiRenderPass::CreationInput creationInput = {
                DEFAULT_IMGUI_RENDER_PASS_NAME,
                getInitialGuiElementCallbacks()
        };
        imguiRenderPass = std::make_shared<PGraphics::PImguiRenderPass>(creationInput);
    }

    void CoreMenuEngineMode::showApplicationWindow() {
        auto windowSystem = applicationContext->getWindowSystem();
        auto showWindowResult = windowSystem->showWindow();
        if (showWindowResult != WindowSystem::ShowWindowResult::SUCCESS) {
            throw std::runtime_error("Error during CoreMenuEngineMode::setUp - unable to show application window!");
        }
    }

    EngineMode::RunResult CoreMenuEngineMode::run() {
        // here is where we'd implement the main loop, after all setUp has been completed
        while (true) { // TODO - completely rewrite this to be smarter and use std::chrono, this is just placeholder
            auto prepareNextFrameResult = prepareFrame();
            if (prepareNextFrameResult == PrepareFrameResult::FAILURE) {
                return EngineMode::RunResult::FAILURE;
            }

            if (prepareNextFrameResult == PrepareFrameResult::STOP_RUNNING) {
                break;
            }

            auto drawNextFrameResult = drawFrame();
            if (drawNextFrameResult != DrawFrameResult::SUCCESS) {
                return EngineMode::RunResult::FAILURE;
            }
        }

        return EngineMode::RunResult::SUCCESS;
    }

    PEngineMode::PrepareFrameResult CoreMenuEngineMode::prepareFrame() {
        // TODO - implement core menu engine mode
        return PEngineMode::PrepareFrameResult::STOP_RUNNING;
    }

    PEngineMode::DrawFrameResult CoreMenuEngineMode::drawFrame() {
        // TODO - re-implement once prerequisite functionality is in place
        try {
            auto drawNextFrameResult = engineCore->getGraphicsEngine()->drawNextFrame();
            return (drawNextFrameResult == GraphicsEngine::DrawFrameResult::SUCCESS)
                   ? PEngineMode::DrawFrameResult::SUCCESS : PEngineMode::DrawFrameResult::FAILURE;
        }
        catch (std::exception &exception) {
            return PEngineMode::DrawFrameResult::FAILURE;
        }
    }

}// namespace PEngine