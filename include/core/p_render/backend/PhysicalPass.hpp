#pragma once

#include "../PRender.hpp"

#include "./Context.hpp"

#include "../render_graph/RenderGraph.hpp"

#include "../render_graph/resources/RenderResource.hpp"

namespace backend {

class Program;

class PhysicalPass {
  public:
    enum RenderPassOp // these will be hooked in later
    {
        RENDER_PASS_OP_CLEAR_DEPTH_STENCIL_BIT = 1 << 0,
        RENDER_PASS_OP_LOAD_DEPTH_STENCIL_BIT = 1 << 1,
        RENDER_PASS_OP_STORE_DEPTH_STENCIL_BIT = 1 << 2,
        RENDER_PASS_OP_DEPTH_STENCIL_READ_ONLY_BIT = 1 << 3,
        RENDER_PASS_OP_ENABLE_TRANSIENT_STORE_BIT = 1 << 4,
    };
    using RenderPassOpFlags = uint32_t;

    struct SubpassAttachments {
        std::shared_ptr<std::vector<VkAttachmentReference>> colorAttachments = {};
        std::shared_ptr<std::vector<VkAttachmentReference>> inputAttachments = {};
        std::shared_ptr<std::vector<VkAttachmentReference>> resolveAttachments = {};
    };

    struct Barrier {

    };

    struct PhysicalSubpass {
      // index within PhysicalPass
      unsigned int index = ~0u;

      // Program 
      std::shared_ptr<Program> program; // this should wrap everything up for 

      // graphics pipeline to be created
      VkPipeline pipeline = VK_NULL_HANDLE;

      // some depth/stencil information is probably important
        // we need to know whether we have depth/stencil enabled and also 
        // whether we're writing to it
      bool depthStencilTestEnabled = false;
      bool depthStencilWriteEnabled = false;

      unsigned int colorAttachmentCount = 0;

      // TODO - attach geometry from the Scene so that subpasses can bind their resp. vertex/index buffers etc
      
    };

    PhysicalPass(std::vector<unsigned int> &passIndices, std::shared_ptr<RenderGraph> graph, std::shared_ptr<Context> context);
    ~PhysicalPass() = default;

    void execute();

  private:
    std::shared_ptr<Context> context_;
    std::shared_ptr<RenderGraph> graph_;

    // physical pass vulkan state
    VkDevice device_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;

    std::vector<VkImageView> imageViews_ = {}; // i guess i'll just have each pass build the image views for now
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

    // each physical pass should have a vector of subpasses that can be iterated through and
    // executed 
    std::vector<std::shared_ptr<PhysicalSubpass>> physicalSubpasses_ = {};

    // we may need to insert layout transition barriers for some resources
      // maybe i'll try storing a map from the subpass index to a barrier pointer 
    std::vector<std::shared_ptr<Barrier>> barriers_ = {};
    std::map<unsigned int, unsigned int> subpassBarrierIndices = {};
 
};

}