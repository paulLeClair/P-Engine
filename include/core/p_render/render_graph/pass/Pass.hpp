#pragma once

//FEB21 hmm... so i'm gonna try and base my Pass abstraction around the Vulkan render pass
  // each render graph NODE will correspond to a VULKAN RENDER PASS, complete with a list of Subpasses
  // that are direct analogues to Vulkan subpasses as well 

// MAR30 
  // thread pool includes
#include "../../../../core/thread_pool/job_queue/JobQueue.hpp"

// subpass
// #include "./Subpass.hpp"

// render resource includes
// #include "../resources/ImageResource.hpp"
// #include "../resources/BufferResource.hpp"
#include "../resources/RenderResource.hpp"

#include "../../PRender.hpp"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

class RenderGraph;
class Subpass;

class Pass : public std::enable_shared_from_this<Pass> {
  public:
    Pass(const std::string &name, unsigned int index, std::shared_ptr<ThreadPool> pool, std::shared_ptr<RenderGraph> graph, ResourceQueueUsageFlags usages = (uint32_t)ResourceQueueUsageFlagBits::GRAPHICS_QUEUE); 
    ~Pass() = default;

    struct AccessedResource {
        VkPipelineStageFlags stages = 0;
        VkAccessFlags flags = 0;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    struct AccessedImageResource : AccessedResource {
        ImageResource *resource = nullptr;
    };

    struct AccessedBufferResource : AccessedResource {
        BufferResource *resource = nullptr;
    };

    // management functions
    void bake();
    bool validate() const;
    void execute();
    void reset();

    // access functions
    const std::string &getName() const;
    const unsigned int &getIndex() const;
    
    // MAY28 - functions for setting up shaders for the pass
        // i think we can just have a "global" graph registry of pre-compiled shaders, 
        // and the user can add them to the registry as needed
    // i think i already have CMake compiling shaders offline, so i think it should mostly just be 
    // on the user to specify the shaders they want by name;
        // since we need to provide a higher-level view where we simply hook in virtual render graph resources,
        // i'll have to implement a RenderGraph::Shader type thing probably; 
            // this can just maintain resources + whatever else needed to build pipelines, 
        // resources should be able to be hooked into a PRender::Scene, which in turn
            // should be controlled by external resources to actually draw some graphics
    void setVertexShader(const std::string &name);
    void setFragmentShader(const std::string &name);
    void setTessellationEvaluationShader(const std::string &name);
    void setTessellationControlShader(const std::string &name);
    void setGeometryShader(const std::string &name);

    // will probably need some functions for hooking up passes to the scene object 
    
    // functions for setting up subpasses
    // void addSubpass(const std::string &name, std::shared_ptr<Subpass> subpass); // i guess subpasses can be created as shared pointers
    std::shared_ptr<Subpass> addSubpass(const std::string &subpassName); // this should create and initialize an empty Graph::Subpass
    void removeSubpass(const std::string &name);
    
    // then the process can be:
        // create your Pass
            // create your subpasses
                // add all resources used by the subpass
                // specify shaders + bindings for shader resources (needs to line up with shader code)

    // JUL9 - now that we're basing everything around subpasses, it might make more sense
    // to have the pass be kind of an "interface" to some set of Graph::Subpasses, which aren't directly accessed
        // since i'm trying to base everything around the subpasses, we should probably specify the subpass we're adding resources to

    // Themaister has only an "addColorOutput" function which just takes an input resource name argument 
    ImageResource &addColorOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName = "");
    
    // depth/stencil inputs/outputs
    ImageResource &addDepthStencilInput(const std::string &subpass, const std::string &inputName, const AttachmentInfo &attachmentInfo);
    ImageResource &addDepthStencilOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo);

    // texture / storage inputs/outputs
    ImageResource &addAttachmentInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);
    // ImageResource &addStorageTextureOutput(const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName = "");
    // ImageResource &addTextureInput(const std::string &name, VkPipelineStageFlags stages);

    // storage image
    ImageResource &addStorageImageInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);
    ImageResource &addStorageImageOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName="");

    // textures
    ImageResource &addTextureInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);

    // vertex / index buffers
        // i think i'm gonna handle geometry differently, using the Scene thing
    // BufferResource &addVertexBuffer(const std::string &name);
    // BufferResource &addIndexBuffer(const std::string &name);

    // uniform buffers
    BufferResource &addUniformBufferInput(const std::string &subpass, const std::string &inputName, const BufferInfo &bufferInfo); // not sure what args to pass for inputs
    BufferResource &addUniformTexelBufferInput(const std::string &subpass, const std::string &inputName, const BufferInfo &bufferInfo);

    // storage buffers
    BufferResource &addStorageBufferInput(const std::string &subpass, const std::string &inputName, const BufferInfo &bufferInfo);
    BufferResource &addStorageBufferOutput(const std::string &subpass, const std::string &outputName, const BufferInfo &bufferInfo, const std::string &inputName = "");
    BufferResource &addStorageTexelBufferInput(const std::string &subpass, const std::string &inputName, const BufferInfo &bufferInfo);
    BufferResource &addStorageTexelBufferOutput(const std::string &subpass, const std::string &outputName, const BufferInfo &bufferInfo, const std::string &inputName = "");

    std::vector<std::shared_ptr<Subpass>> getSubpasses() const {
        return subpasses_;
    }

    const std::vector<ImageResource*> &getColorInputs() {
        return colorInputs_;
    }

    const std::vector<ImageResource*> &getColorOutputs() {
        return colorOutputs_;
    }

    // storage images
    const std::vector<ImageResource*> &getStorageImageInputs() {
        return storageImageInputs_;
    }

    const std::vector<ImageResource*> &getStorageImageOutputs() {
        return storageImageOutputs_;
    }

    // depth/stencil (can only have 1 input/output per pass)
    ImageResource *getDepthStencilInput() const {
        // return (depthStencilInput_) ? *depthStencilInput_ : nullptr;
        return depthStencilInput_;
    }
    ImageResource *getDepthStencilOutput() const {
        // return (depthStencilOutput_) ? *depthStencilOutput_ : nullptr;
        return depthStencilOutput_;
    }

    // storage buffers
    const std::vector<BufferResource*> &getStorageBufferInputs() {
        return storageBufferInputs_;
    }
    const std::vector<BufferResource*> &getStorageBufferOutputs() {
        return storageBufferOutputs_;
    }

    // multisample resolve outputs
    const std::vector<ImageResource*> &getResolveOutputs() {
        return resolveOutputs_;
    }

    // texture + attachment inputs
    const std::vector<ImageResource*> &getAttachmentInputs() {
        return attachmentInputs_;
    }

    const std::vector<ImageResource*> &getTextureInputs() {
        return textureInputs_;
    }

    const std::vector<BufferResource*> &getUniformBuffers() {
        return uniformBuffers_;
    }

    const std::vector<BufferResource*> &getUniformTexelBuffers() {
        return uniformTexelBuffers_;
    }

    const std::vector<BufferResource*> &getStorageTexelBufferInputs() {
        return storageTexelBufferInputs_;
    }

    const std::vector<BufferResource*> &getStorageTexelBufferOutputs() {
        return storageTexelBufferOutputs_;
    }

  private:

    // name of the pass in the render graph (mandatory)
    std::string name_;

    unsigned int index_;

    // thread pool handle
    std::shared_ptr<ThreadPool> pool_;

    // render graph handle (kinda redundant to pass the pool handle hwen it could just get it from the graph, but w/e)
    // RenderGraph &graph_;
    std::shared_ptr<RenderGraph> graph_;

    // baked status (gonna try having that mean that a physical pass has been set up and the pass can be executed)
    // bool baked_ = false;

    ResourceQueueUsageFlags queueUsages_ = 0;

    std::vector<std::shared_ptr<Subpass>> subpasses_ = {}; // should each subpass maintain info about which resources it uses?
    std::map<std::string, unsigned int> subpassNames_;

    /* PASS RESOURCES */
    // JUL12 - i'm gonna commit to trying a system where you specify attachments to the overall pass,
    // and the subpasses themselves only maintain which names of which resources they use 
        // this should make it fairly easy to allow multiple resource usage types in a pass, and
        // the specific subpasses which use them in whichever specific ways are specified when you build
        // the graph
    

    // attachment images
    std::vector<ImageResource*> attachmentInputs_;
    std::unordered_map<std::string, unsigned int> attachmentInputNames_;

    // color (images)
    std::vector<ImageResource*> colorInputs_;
    std::unordered_map<std::string, unsigned int> colorInputNames_;

    std::vector<ImageResource*> colorOutputs_;
    std::unordered_map<std::string, unsigned int> colorOutputNames_;

    // depth/stencil (can only have 1 input/output per pass)
        // this means that all subpasses have only these depth stencil input/output options
    ImageResource *depthStencilInput_ = nullptr;
    std::string depthStencilInputName_ = "";

    ImageResource *depthStencilOutput_ = nullptr;
    std::string depthStencilOutputName_ = ""; 

    // storage images
    std::vector<ImageResource*> storageImageInputs_;
    std::unordered_map<std::string, unsigned int> storageImageInputNames_;

    std::vector<ImageResource*> storageImageOutputs_;
    std::unordered_map<std::string, unsigned int> storageImageOutputNames_;

    // multisample resolve outputs
    std::vector<ImageResource*> resolveOutputs_;
    std::unordered_map<std::string, unsigned int> resolveOutputNames_;

    // texture inputs
    std::vector<ImageResource*> textureInputs_;
    std::unordered_map<std::string, unsigned int> textureInputNames_;

    // uniform buffers
    std::vector<BufferResource*> uniformBuffers_;
    std::unordered_map<std::string, unsigned int> uniformBufferNames_;

    std::vector<BufferResource*> uniformTexelBuffers_;
    std::unordered_map<std::string, unsigned int> uniformTexellBufferNames_;

    // storage buffers
    std::vector<BufferResource*> storageBufferInputs_;
    std::unordered_map<std::string, unsigned int> storageBufferInputNames_;

    std::vector<BufferResource*> storageBufferOutputs_;
    std::unordered_map<std::string, unsigned int> storageBufferOutputNames_;

    std::vector<BufferResource*> storageTexelBufferInputs_;
    std::unordered_map<std::string, unsigned int> storageTexelBufferInputNames_;

    std::vector<BufferResource*> storageTexelBufferOutputs_;
    std::unordered_map<std::string, unsigned int> storageTexelBufferOutputNames_;

    // TODO: add push constants
};