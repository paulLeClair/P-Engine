#pragma once

#include <memory>
#include "../../GraphicsPipelineIR/GraphicsPipelineIR.hpp"
#include "../RenderPassIR.hpp"

namespace pEngine::girEngine::gir::renderPass {

    /**
     * This should correspond to the Vulkan 1.3 render passes that don't use framebuffers and what not.
     *
     * Okay, now that we're on scene bake, we can start filling in this class.
     *
     * Chances are it should probably just closely match the scene interface for the time being,
     * although obviously we can probably change a lot of the specifics
     */
    class DynamicRenderPassIR : public RenderPassIR {
    public:

        struct CreationInput : public RenderPassIR::CreationInput {
            // TODO - add any additional information that's required for these dynamic render passes
        };

        explicit DynamicRenderPassIR(const CreationInput &creationInput)
                : RenderPassIR(creationInput) {

        }

        ~DynamicRenderPassIR() override = default;

    private:
        // TODO - add any additional information that's required for these dynamic render passes

    };

}