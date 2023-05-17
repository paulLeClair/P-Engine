#pragma once

#include <vector>

//#include "./RenderPass/RenderPass.hpp"
//#include "../SceneResources/Buffer/Buffer.hpp"
//#include "../SceneResources/Image/Image.hpp"

class RenderPass;

class Buffer;

class Image;

class SceneRenderGraph {
public:
    ~SceneRenderGraph() = default;

    enum class AddNextRenderPass {
        SUCCESS,
        FAILURE
    };

    virtual AddNextRenderPass addNextRenderPass(const std::shared_ptr<RenderPass> &nextRenderPass) = 0;

    virtual std::shared_ptr<RenderPass> getRenderPass(const std::string &name) = 0;

    virtual std::vector<std::shared_ptr<RenderPass>> &getRenderPasses() = 0;

    enum CreateResourceResult {
        SUCCESS,
        FAILURE
    };

    virtual CreateResourceResult createBuffer(std::shared_ptr<Buffer> &buffer) = 0;

    virtual CreateResourceResult createImage(std::shared_ptr<Image> &image) = 0;

};
