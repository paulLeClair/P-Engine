//
// Created by paull on 2023-10-15.
//

#pragma once

#include <vector>
#include "../GraphicsIntermediateRepresentation.hpp"
#include "../RenderPassIR/RenderPassIR.hpp"
#include "../RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp"
#include "../RenderPassIR/DearImguiRenderPassIR/DearImguiRenderPassIR.hpp"

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
    struct RenderGraphIR : public GraphicsIntermediateRepresentation {
        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            // TODO - probably add swapchain configuration stuff here;
            //  I'm not sure exactly how top-level I should make the swapchain but it could potentially work as per-graph

            // for the single-animated-model demo, a very rigid structure for render passes will exist (temporary tho)
            std::vector<DynamicRenderPassIR> dynamicRenderPasses;
            dearImgui::DearImguiRenderPassIR dearImguiRenderPass;
        };

        explicit RenderGraphIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  dynamicRenderPasses(creationInput.dynamicRenderPasses) {
        }

        ~RenderGraphIR() override = default;

        /**
         * I think this should just maintain the ordering of render passes basically and aggregate them all up
         */
        std::vector<DynamicRenderPassIR> dynamicRenderPasses = {};

        boost::optional<gir::renderPass::dearImgui::DearImguiRenderPassIR> imguiRenderPass = boost::none;
    };
} // gir
