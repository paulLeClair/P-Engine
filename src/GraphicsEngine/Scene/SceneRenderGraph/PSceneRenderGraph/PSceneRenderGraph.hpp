#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../SceneRenderGraph.hpp"
#include "../RenderPass/RenderPass.hpp"
#include "../../SceneResources/Buffer/Buffer.hpp"


namespace PGraphics {

    class PSceneRenderGraph : public SceneRenderGraph {
    public:
        PSceneRenderGraph() = default;

        ~PSceneRenderGraph() = default;

        AddNextRenderPass addNextRenderPass(const std::shared_ptr<RenderPass> &renderPass) override;

        std::shared_ptr<RenderPass> getRenderPass(const std::string &name) override;

        std::vector<std::shared_ptr<RenderPass>> &getRenderPasses() override;

        CreateResourceResult createBuffer(std::shared_ptr<Buffer> &buffer) override;

        CreateResourceResult createImage(std::shared_ptr<Image> &image) override;

    private:
        std::vector<std::shared_ptr<RenderPass>> renderPasses;
        std::unordered_map<std::string, unsigned long> renderPassNamesToIndices;

        std::vector<std::shared_ptr<Buffer>> buffers;
        std::unordered_map<std::string, unsigned long> bufferNamesToIndices;

        std::vector<std::shared_ptr<Image>> images;
        std::unordered_map<std::string, unsigned long> imageNamesToIndices;

    };

}// namespace PGraphics
