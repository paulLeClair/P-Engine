#pragma once

#include "ApplicationContext/ApplicationContext.hpp"
#include "Renderer/Renderer.hpp"
#include "../Scene/Scene.hpp"

namespace pEngine::girEngine::backend {
    class GraphicsBackend {
    public:
        virtual ~GraphicsBackend() = default;

        // TODO - evaluate whether we need this
        enum class DrawFrameResult {
            SUCCESS,
            FAILURE
        };

        virtual DrawFrameResult drawFrame() = 0;

        virtual std::shared_ptr<appContext::ApplicationContext> getApplicationContext() = 0;

        virtual std::shared_ptr<render::Renderer>
        getFrameContext() = 0;

        enum class BakeResult {
            FAILURE,
            SUCCESS
        };

        virtual BakeResult
        bakeGirs(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &girList) = 0;

    };
}
