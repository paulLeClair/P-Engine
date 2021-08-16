#pragma once

// job queue
#include "../../../core/thread_pool/job_queue/JobQueue.hpp"

#include "../PRender.hpp"
#include "../../../vulkan_memory_allocator/vk_mem_alloc.h"

#include "./resources/ResourceDimensions.hpp"

#include "../backend/resources/Resource.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include <assert.h>

class Pass;
class RenderResource;
class ImageResource;
class BufferResource;
class Shader;

namespace Backend {
    class Context;
    class FrameContext;
    class PhysicalPass;
    struct PhysicalPassCreateInfo;
    class ShaderModule; 
}

class RenderGraph : public std::enable_shared_from_this<RenderGraph> {
  public:
    RenderGraph(const std::string &name,  std::shared_ptr<ThreadPool> pool, std::shared_ptr<Backend::Context> context, VmaAllocator allocator);
    ~RenderGraph();

    /* Render Graph Interface */
    
    // access functions
    Pass &getPass(const std::string &name);

    void setBackbufferSource(std::string name) {
        if (!resourceNames_.count(name)) {
            throw std::runtime_error("");
        }

        backbufferResourceName_ = name;
    }
    
    std::shared_ptr<Pass> appendPass(const std::string &passName);

    // resources etc 
      // gotta change these to return shared pointers too
    ImageResource &getImageResource(const std::string &name, const AttachmentInfo *info = nullptr);
    BufferResource &getBufferResource(const std::string &name, const BufferInfo *info = nullptr);
    
    // shaders!
      // unlike the above get* functions, we'll return the shared pointer to the shader module
    std::shared_ptr<Backend::ShaderModule> getShaderModule(const std::string &shaderFilename);

    // physical resources!
    ResourceDimensions getResourceDimensions(BufferResource &resource) const;
    ResourceDimensions getResourceDimensions(ImageResource &resource) const;
    std::vector<std::shared_ptr<Backend::Resource>> &getPhysicalResources() {
      assert(physicalResources_.size()); // shouldn't call this before physical resources have been built
        // and we should always have at least the swapchain images to work with
      return physicalResources_;
    }

    // swapchain-specific stuff
    std::shared_ptr<ResourceDimensions> getSwapchainDimensions() {
      if (swapchainDimensions_) {
        return swapchainDimensions_;
      }
      
      ResourceDimensions dim;
      dim.name = "swapchain";

      dim.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // gonna try all 3, but 
        // might only need VK_IMAGE_USAGE_COLOR_ATTAChMENT_BIT 
      
      dim.layers = 1;
      dim.mipLevels = 1;
      dim.format = context_->WSI().getSwapchainImageFormat();

      dim.depth  = 1;
      dim.width = context_->WSI().getSwapchainImageSize().width;
      dim.height = context_->WSI().getSwapchainImageSize().height;
      
      dim.queueUsages = ResourceQueueUsageFlagBits::GRAPHICS_QUEUE;

      swapchainDimensions_ = std::make_shared<ResourceDimensions>(dim);
      return swapchainDimensions_;
    }

    // bake function
    void bake();

    // functions for actually rendering a frame, provided the render graph has been finalized
    void execute(Backend::FrameContext &frame); // execute the graph if finalized, enqueuing a submit(CommandBuffer) to the given submission queue

    void reset(); // reset all passes if finalized

    std::vector<std::shared_ptr<Pass>> &getPasses() {
      return passes_;
    }

  private:
    /* Render Graph State */
    bool baked_ = false;

    std::string name_ = "";

    std::shared_ptr<Backend::Context> context_;

    std::shared_ptr<ThreadPool> pool_;

    std::vector<std::shared_ptr<Pass>> passes_; // gonna use shared pointers for this actually, passes can exist as their own
    std::map<std::string, unsigned int> passNames_; 

    // high-level resources that have been registered with the graph + its passes
    std::vector<std::shared_ptr<RenderResource>> resources_;
    std::map<std::string, unsigned int> resourceNames_;

    // backbuffer source 
    std::string backbufferResourceName_; // this is used to terminate the graph

    // pass stack + merge/dependency info for each pass
    std::vector<unsigned int> passStack_;
    std::vector<std::unordered_set<unsigned int>> passDependencies_;
    std::vector<std::unordered_set<unsigned int>> passMergeDependencies_;

    /* Physical Resources */ 
    // vulkan memory allocator stuff:
    VmaAllocator allocator_;

    // physical resource dimensions
    std::vector<ResourceDimensions> physicalResourceDimensions_;
    std::shared_ptr<ResourceDimensions> swapchainDimensions_; 

    // physical resources 
    std::vector<std::shared_ptr<Backend::Resource>> physicalResources_; // these should just built 1 resource for each of the ResourceDimensions

    // physical passes (built up at bake time, should be possible to draw the frame
    // by just executing these one by one)
    std::vector<std::shared_ptr<Backend::PhysicalPass>> physicalPasses_;

    std::vector<std::shared_ptr<Backend::ShaderModule>> shaderModules_;
    std::map<std::string, unsigned int> shaderModuleNames_;
    
    /* Render Graph Internal Functions */

    // BAKING UTILITIES
    void validatePasses(); // this should just do some simple sanity checking, right now it's light but should be extended with time

    void analyzeDependencies(); 
        void traverseDependencies(Pass &pass, unsigned int stackCounter);
        void recursePassDependencies(const Pass &self, std::unordered_set<unsigned int> &writtenPasses, unsigned int stackCounter, bool noCheck, bool ignoreSelf, bool mergeDependency);
        bool dependsOnPass(unsigned int destinationPass, unsigned int sourcePass);

    void filterPasses();

    void reorderPasses();

    void buildPhysicalResources();

    void buildPhysicalPasses();
      void setupAttachments(Pass &pass, Backend::PhysicalPassCreateInfo &info);
      void setupSubpasses(Pass &pass, Backend::PhysicalPassCreateInfo &info);

    void buildPrograms();

    void buildBarriers(); 

    void buildAliases();
    
};
