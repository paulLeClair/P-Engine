#pragma once

#include "Backend/Backend.hpp"
#include "Scene/Scene.hpp"

class GraphicsEngine {
public:
    ~GraphicsEngine() = default;

    virtual std::shared_ptr<Scene> getScene() = 0;

    virtual std::shared_ptr<Backend> getBackend() = 0;

    enum class BakeResult {
        SUCCESS,
        FAILURE
    };

    virtual BakeResult bakeRenderData() = 0;

    enum class DrawFrameResult {
        SUCCESS,
        FAILURE
    };

    /**
     * Current idea for this one is that inside an EngineMode::run() implementation,
     * you'll do all your updates as part of a "prepareFrame()" function, and then after that
     * you call this function to draw the next frame with all updates committed
     */
    virtual DrawFrameResult drawNextFrame() = 0;

};
