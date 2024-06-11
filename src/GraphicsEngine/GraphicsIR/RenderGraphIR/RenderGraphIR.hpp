//
// Created by paull on 2023-10-15.
//

#pragma once

#include <vector>
#include "../GraphicsIntermediateRepresentation.hpp"
#include "../RenderPassIR/RenderPassIR.hpp"

using namespace pEngine::girEngine::gir::renderPass;

namespace pEngine::girEngine::gir {
    /**
     * New idea: aggregating render passes (and probably graphics pipelines)
     * under the umbrella of a RenderGraphIR class.
     *
     * This means that we don't have to do anything like have each render pass store its
     * place in the overall ordering of render passes, overall reduces slightly the
     * amount of work the backend has to do in processing the big list of GIRs it gets handed.
     */
    class RenderGraphIR : public GraphicsIntermediateRepresentation {
    public:
        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            // TODO - probably add swapchain configuration stuff here;
            //  I'm not sure exactly how top-level I should make the swapchain but it could potentially work as per-graph

            std::vector<std::shared_ptr<RenderPassIR> > renderPasses;
        };

        explicit RenderGraphIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  renderPasses(creationInput.renderPasses) {
        }

        ~RenderGraphIR() override = default;

        [[nodiscard]] const std::vector<std::shared_ptr<RenderPassIR>> &getRenderPasses() const {
            return renderPasses;
        }

    private:
        /**
         * I think this should just maintain the ordering of render passes basically and aggregate them all up
         */
        std::vector<std::shared_ptr<RenderPassIR> > renderPasses = {};
    };
} // gir
