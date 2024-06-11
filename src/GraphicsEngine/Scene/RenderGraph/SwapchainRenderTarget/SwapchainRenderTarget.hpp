//
// Created by paull on 2023-09-18.
//

#pragma once

#include <memory>

namespace pEngine::girEngine::scene::graph {

    class RenderingAttachment;

    /**
     * Since each render graph essentially outlines one particular graphics pipeline,
     * maybe it will work to have configurability for the actual render target image(s)
     * here...?
     *
     * The idea is that you specify a particular render target image (with resolution and
     * other such relevant information), and then rather than create an Image for it manually
     * you just specify this thing and then upon scene bake it will generate everything for you
     *
     *
     */
    class SwapchainRenderTarget {
    public:
        // TODO - actually design this

        std::shared_ptr<RenderingAttachment> getRenderingAttachment() {

            return nullptr;
        }
    };

} // scene
