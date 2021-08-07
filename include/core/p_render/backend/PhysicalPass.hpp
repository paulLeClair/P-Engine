#pragma once

// this i guess will contain some  for implementing a physical pass
    // i'm gonna have to implement some more stuff in the Vulkan backend to continue down this path towards a 3D scene

// i think this can be a simpler construct can just be used to describe which of the pass' resources
    // are used in the current subpass;
    
    // i think it might even be able to just wrap up VkSubpassDependency type things, plus probably some execution stuff

    // really, i think all high-level rendering code should have to be registered in one or more high-level subpasses;
    // they can store indices into their parent passes of resources being used, and 
    // when building the high-level render pass you should have to package up all rendering into one or more subpasses
      // that way hopefully it can merge them behind the scenes if you want, whether you spread things out over multiple
      // passes or if you package things up in subpasses yourself
    
    // iirc the idea will be to provide a callback for filling the threadpool queue with functions that
    // should result in command buffers being recorded during subpass execution
      // eg, if you have 100 meshes, add 100 "drawMesh(...) = [&...](...){}" jobs, and each thread
      // will execute one lambda to do some useful rendering work;

    // i'm fairly certain that you can still package up complicated tasks like this, but it may be 
    // useful to add some other callbacks for more specific things (we'll see, i'd like to KISS) 


#include "../PRender.hpp"

#include "./Context.hpp"

#include "../render_graph/RenderGraph.hpp"

#include "../render_graph/resources/RenderResource.hpp"

// #ifndef Unused
// #define Unused ~0u
// #endif

// but hopefully i can hack together something that works :)
namespace Backend {

// class Barrier;
// class ImageView;

// TODO:
  // figure out how to use these... might just simplify and get rid of em if possible
// namespace Request {
//     struct ColorClearRequest {

//     };

//     struct DepthClearRequest {

//     };

//     struct ScaledClearRequest {

//     };

//     struct MipmapRequest {

//     };
// }

// depth/stencil usage enum 
// enum DepthStencilMode {
//     None,
//     ReadOnly,
//     ReadWrite
// };



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

    // this should probably actually be used as the final struct that 
    // will be maintained for PhysicalPass execution (well, an array of them, one per Graph::Subpass)
      // it'll contain the information required to have multithreaded command buffer recording
      // occur 
        // should be stuff like:
        // - the Backend::Program that corresponds to the shaders + resources setup for this subpass
          // contains:
            // - all the DSLs + push constant stuff for a PipelineLayout
            // - all the shader info for setting up graphics pipeline shader stages
        // - the graphics pipeline that can be bound
        // - maybe a descriptor pool? plus the actual descriptor sets 
    struct PhysicalSubpass {
      // TODO: set this up so that we can fill it using a Graph::Subpass + other physical pass info
        // we'll probably want to take a few things directly from the Subpass...
          // eg the callbacks for recording command buffers etc

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

      // whatever else a physical subpass needs!

    };

    // can potentially un-delete those inline functions if they turn out to be necessary

    // PhysicalPass(PhysicalPassCreateInfo &info); // gonna redo this
    // PhysicalPass(std::vector<unsigned int> passIndices, std::vector<std::shared_ptr<Pass>> &passes); // gonna try building the entire physical pass in the ctor off of 
    PhysicalPass(std::vector<unsigned int> passIndices, std::shared_ptr<RenderGraph> graph, std::shared_ptr<Context> context);
    ~PhysicalPass() = default;

    void execute();

  private:
    std::shared_ptr<Context> context_;
    std::shared_ptr<RenderGraph> graph_;

    // physical pass vulkan state
    VkDevice device_ = VK_NULL_HANDLE;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;

    // honestly, i think it makes sense to have physical passes create and store their own Framebuffers,
    // and we can also look for compatible framebuffers to reuse...
      // we bind the actual framebuffer when we begin the render pass, so that's where that should happen
    std::vector<VkImageView> imageViews_ = {}; // i guess i'll just have each pass build the image views for now
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

    // each physical pass should have a vector of subpasses that can be iterated through and
    // executed 
    std::vector<std::shared_ptr<PhysicalSubpass>> physicalSubpasses_ = {};

};

}