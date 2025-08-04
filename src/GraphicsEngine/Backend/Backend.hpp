#pragma once

#include "ApplicationContext/ApplicationContext.hpp"
#include "Renderer/Renderer.hpp"
#include "../Scene/Scene.hpp"

namespace pEngine::girEngine::backend {
    class GraphicsBackend {
    public:
        virtual ~GraphicsBackend() = default;

        enum class DrawFrameResult {
            SUCCESS,
            FAILURE
        };

        virtual DrawFrameResult drawFrame() = 0;

        // TODO -> probably rip out this one too
        virtual std::shared_ptr<appContext::ApplicationContext> getApplicationContext() = 0;

        enum class BakeResult {
            FAILURE,
            SUCCESS
        };

        virtual BakeResult
        bakeGirs(const gir::generator::GirGenerator::BakeOutput &girList) = 0;

    };
}
