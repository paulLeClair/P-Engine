#pragma once

#include "ApplicationContext/ApplicationContext.hpp"
#include "FrameExecutionController/FrameExecutionController.hpp"
#include "../Scene/Scene.hpp"

class Backend {
public:
    ~Backend() = default;

    enum class DrawFrameResult {
        SUCCESS,
        FAILURE
    };

    virtual void bakeRenderData() = 0;

    virtual DrawFrameResult drawFrame() = 0;

    virtual std::shared_ptr<ApplicationContext> getApplicationContext() = 0;

    virtual std::shared_ptr<FrameExecutionController> getFrameExecutionController() = 0;
};
