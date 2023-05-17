//
// Created by paull on 2022-06-14.
//

#include "PSceneRenderGraph.hpp"
#include "../../SceneResources/Image/Image.hpp"
#include "../../SceneResources/Buffer/Buffer.hpp"

namespace PGraphics {

    PSceneRenderGraph::AddNextRenderPass
    PSceneRenderGraph::addNextRenderPass(const std::shared_ptr<RenderPass> &renderPass) {
        if (renderPass->getName().empty()) {
            return AddNextRenderPass::FAILURE;
        }

        renderPasses.push_back(renderPass);
        renderPassNamesToIndices[renderPass->getName()] = renderPasses.size() - 1;
        return AddNextRenderPass::SUCCESS;
    }

    std::shared_ptr<RenderPass> PSceneRenderGraph::getRenderPass(const std::string &name) {
        if (name.empty() || renderPassNamesToIndices.count(name) == 0) {
            return nullptr;
        }

        return renderPasses[renderPassNamesToIndices[name]];
    }

    std::vector<std::shared_ptr<RenderPass>> &PSceneRenderGraph::getRenderPasses() {
        return renderPasses;
    }

    SceneRenderGraph::CreateResourceResult PSceneRenderGraph::createBuffer(std::shared_ptr<Buffer> &buffer) {
        if (bufferNamesToIndices.count(buffer->getName())) {
            return SceneRenderGraph::CreateResourceResult::FAILURE;
        }

        buffers.push_back(buffer);
        bufferNamesToIndices[buffer->getName()] = buffers.size() - 1;
        return SceneRenderGraph::CreateResourceResult::SUCCESS;
    }

    SceneRenderGraph::CreateResourceResult PSceneRenderGraph::createImage(std::shared_ptr<Image> &image) {
        if (imageNamesToIndices.count(image->getName())) {
            return SceneRenderGraph::CreateResourceResult::FAILURE;
        }

        images.push_back(image);
        imageNamesToIndices[image->getName()] = images.size() - 1;
        return SceneRenderGraph::CreateResourceResult::SUCCESS;
    }

}// namespace PGraphics