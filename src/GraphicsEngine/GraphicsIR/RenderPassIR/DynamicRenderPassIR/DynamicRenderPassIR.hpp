#pragma once

#include <memory>
#include "../../GraphicsPipelineIR/GraphicsPipelineIR.hpp"
#include "../RenderPassIR.hpp"

using namespace pEngine::util;


namespace pEngine::girEngine::gir::renderPass {
    /**
     * This should correspond to the Vulkan 1.3 render passes that don't use framebuffers and what not.
     */
    class DynamicRenderPassIR final : public RenderPassIR {
    public:
        /* DYNAMIC DATA -> is updated each frame in an ordering determined by user-specified priority values */
        struct DynamicResourceBinding {
            UniqueIdentifier resourceId;

            std::function<std::vector<uint8_t>()> updateCallback = {};

            float priority = 1.0f;
        };

        struct CreationInput : RenderPassIR::CreationInput {
            // TODO - add any additional information that's required for these dynamic render passes

            DynamicRenderPassPipelineStateIR dynamicRenderPassState;

            std::vector<UniqueIdentifier> staticResources = {};

            // TODO -> any other resource types that we want to be dynamic
            std::vector<DynamicResourceBinding> dynamicUniformBuffers = {};
            std::vector<DynamicResourceBinding> dynamicImages = {};
        };

        explicit DynamicRenderPassIR(const CreationInput &creationInput)
            : RenderPassIR(creationInput), dynamicRenderPassState(creationInput.dynamicRenderPassState),
              staticResources(creationInput.staticResources),
              dynamicUniformBuffers(creationInput.dynamicUniformBuffers),
              dynamicImages(creationInput.dynamicImages) {
        }

        ~DynamicRenderPassIR() override = default;

        // TODO - add any additional information that's required for these dynamic render passes

        DynamicRenderPassPipelineStateIR dynamicRenderPassState;

        // NEW: render passes track which resources are to be updated each frame alongside
        // the actual update callback and a priority value to be processed by the backend
        // NOTE: for now this is 1-1 exact match with the front-end
        /* STATIC DATA -> doesn't need to be updated after initialization, exists until scene is closed */
        // in the future maybe I'll add something specific here, but for now we can just have any data that isn't
        // specifically marked as dynamic be automatically lumped into the static grouping
        std::vector<UniqueIdentifier> staticResources = {};

        // TODO -> any other resource types that we want to be dynamic
        std::vector<DynamicResourceBinding> dynamicUniformBuffers = {};
        std::vector<DynamicResourceBinding> dynamicImages = {};
    };
}
