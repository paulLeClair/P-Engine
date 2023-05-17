//
// Created by paull on 2022-08-26.
//

#include "PGraphicsEngine.hpp"

namespace PGraphics {
    GraphicsEngine::BakeResult PGraphicsEngine::bakeRenderData() {
        try {
            backend->bakeRenderData();
        }
        catch (std::exception &exception) {
            return GraphicsEngine::BakeResult::FAILURE;
        }

        return GraphicsEngine::BakeResult::SUCCESS;
    }

    GraphicsEngine::DrawFrameResult PGraphicsEngine::drawNextFrame() {
        // BEFORE actually drawing the next frame, we have to commit any updates that have been made


        auto drawNextFrameResult = backend->drawFrame();
        if (drawNextFrameResult != Backend::DrawFrameResult::SUCCESS) {
            return DrawFrameResult::FAILURE;
        }

        return GraphicsEngine::DrawFrameResult::SUCCESS;
    }
}// namespace PGraphics