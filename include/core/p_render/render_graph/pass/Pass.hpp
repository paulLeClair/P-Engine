#pragma once

#include "../../../../core/thread_pool/job_queue/JobQueue.hpp"

// render resource includes
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
    
    void setVertexShader(const std::string &name);
    void setFragmentShader(const std::string &name);
    void setTessellationEvaluationShader(const std::string &name);
    void setTessellationControlShader(const std::string &name);
    void setGeometryShader(const std::string &name);

    // will probably need some functions for hooking up passes to the scene object 
    
    // functions for setting up subpasses
    std::shared_ptr<Subpass> addSubpass(const std::string &subpassName); // this should create and initialize an empty Graph::Subpass
    void removeSubpass(const std::string &name);
    
    // maybe the process can be:
        // create your Pass
            // create your subpasses
                // add all resources used by the subpass
                // specify shaders + bindings for shader resources (needs to line up with shader code)

    // Themaister has only an "addColorOutput" function which just takes an input resource name argument 
    ImageResource &addColorOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName = "");
    
    // depth/stencil inputs/outputs
    ImageResource &addDepthStencilInput(const std::string &subpass, const std::string &inputName, const AttachmentInfo &attachmentInfo);
    ImageResource &addDepthStencilOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo);

    // texture / storage inputs/outputs
    ImageResource &addAttachmentInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);

    // storage image
    ImageResource &addStorageImageInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);
    ImageResource &addStorageImageOutput(const std::string &subpass, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName="");

    // textures
    ImageResource &addTextureInput(const std::string &subpass, const std::string &name, const AttachmentInfo &attachmentInfo);

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

    // render graph handle (kinda redundant to pass the pool handle hwen it could just get it from the graph, but w/e
    std::shared_ptr<RenderGraph> graph_;

    ResourceQueueUsageFlags queueUsages_ = 0;

    std::vector<std::shared_ptr<Subpass>> subpasses_ = {};
    std::map<std::string, unsigned int> subpassNames_;

    /* PASS RESOURCES */

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

    // TODO: add push constants to the high-level interface
};