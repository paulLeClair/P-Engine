#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../RenderPassIR.hpp"

namespace pEngine::girEngine::gir::renderPass {
/**
 * Stub class for the forseeable future - this will be for the older
 * Vulkan API based on subpasses and render passes, which is mainly
 * useful for GPUs that use tiling (important use case but should be added later)
*/
    class StaticRenderPassIR : public RenderPassIR {
    public:
        struct CreationInput : public RenderPassIR::CreationInput {
            // TODO until dynamic render passes are working
        };

        explicit StaticRenderPassIR(const CreationInput &creationInput)
                : RenderPassIR(creationInput) {

        }

        ~StaticRenderPassIR() override = default;

    private:
        // TODO for now

    };

}