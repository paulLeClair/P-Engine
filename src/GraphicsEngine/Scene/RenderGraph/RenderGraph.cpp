//
// Created by paull on 2025-04-23.
//

#include "RenderGraph.hpp"

namespace pEngine::girEngine::scene::graph {
    void RenderGraph::addDynamicRenderPass(const renderPass::DynamicRenderPass &pass) {
        dynamicRenderPasses.push_back(pass);
    }
}
