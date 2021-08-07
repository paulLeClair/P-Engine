#pragma once

// thread pool
#include "../../../../core/thread_pool/job_queue/JobQueue.hpp"

#include "../../render_graph/Shader.hpp"
#include "../../render_graph/pass/Pass.hpp"

#include <memory>
#include <string>
#include <functional>
#include <assert.h>

// class Pass; 

class Subpass {
  public:
    Subpass(const std::string &name, std::shared_ptr<RenderGraph> graph, std::shared_ptr<ThreadPool> pool, std::shared_ptr<Pass> pass);
    ~Subpass() = default;

    const std::string &getName() const {
        return name_;
    }

    // utilities
    enum SubpassAttachmentUsage {
        ColorAttachment,
        InputAttachment,
        ResolveAttachment,
        DepthStencil,
        UnusedAttachment
    };

    enum SubpassShaderResourceUsage {
        Uniform,
        UniformTexel,
        StorageBufferOutput,
        StorageBufferInput,
        StorageImageOutput,
        StorageImageInput,
        StorageTexelBufferOutput,
        StorageTexelBufferInput,
        SampledImage,
        UnusedShaderResource
    };

    SubpassAttachmentUsage getAttachmentUsage(const std::string &attName) {
        // i think the order that we check these might actually be important, 
        // so i may have to come back to this and think about how i need this to work out

        // hopefully it shouldn't be that big of a deal though

        for (auto &name : attachmentInputs_) {
            if (name == attName) {
                return SubpassAttachmentUsage::InputAttachment;
            }
        }

        for (auto &name : colorInputs_) {
            if (name == attName) {
                return SubpassAttachmentUsage::InputAttachment;
            }
        }

        for (auto &name : colorOutputs_) {
            if (name == attName) {
                return Subpass::ColorAttachment;
            }
        }

        for (auto &name : resolveOutputs_) {
            if (name == attName) {
                return Subpass::ResolveAttachment;
            }
        }

        // check depth/stencil
        if (depthStencilInput_ == attName) {
            return SubpassAttachmentUsage::DepthStencil;
        }

        if (depthStencilOutput_ == attName) {
            return SubpassAttachmentUsage::DepthStencil;
        }

        return SubpassAttachmentUsage::UnusedAttachment;
    }

    SubpassShaderResourceUsage getShaderResourceUsage(const std::string &resName) {
        // same thing but check for shader resources 
        for (auto &name : uniformBuffers_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::Uniform;
            }
        }

        for (auto &name : uniformTexelBuffers_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::UniformTexel;
            }
        }

        for (auto &name : storageBufferOutputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageBufferOutput;
            }
        }

        for (auto &name : storageBufferInputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageBufferInput;
            }
        }

        for (auto &name : storageImageOutputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageImageOutput;
            }
        }

        for (auto &name : storageImageInputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageImageInput;
            }
        }

        for (auto &name : storageTexelBufferOutputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageTexelBufferOutput;
            }
        }

        for (auto &name : storageTexelBufferInputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::StorageTexelBufferInput;
            }
        }

        for (auto &name : textureInputs_) {
            if (name == resName) {
                return SubpassShaderResourceUsage::SampledImage;
            }
        }

        return SubpassShaderResourceUsage::UnusedShaderResource;
    }
  
    std::vector<std::string> &getColorInputs() {
        return colorInputs_;
    }

    std::vector<std::string> &getColorOutputs() {
        return colorOutputs_;
    }

    std::vector<std::string> &getAttachmentInputs() {
        return attachmentInputs_;
    }

    std::vector<std::string> &getResolveOutputs() {
        return resolveOutputs_;
    }

    std::vector<std::string> &getTextureInputs() { // this should correspond to sampled images
        return textureInputs_;
    }

    const std::string &getDepthStencilInput() {
        return depthStencilInput_;
    }

    const std::string &getDepthStencilOutput() {
        return depthStencilOutput_;
    }

    std::vector<std::string> &getUniformBuffers() {
        return uniformBuffers_;
    }
    
    std::vector<std::string> &getUniformTexelBuffers() {
        return uniformTexelBuffers_;
    }

    std::vector<std::string> &getStorageBufferInputs() {
        return storageBufferInputs_;
    }

    std::vector<std::string> &getStorageBufferOutputs() {
        return storageBufferOutputs_;
    }

    std::vector<std::string> &getStorageTexelBufferInputs() {
        return storageTexelBufferInputs_;
    }

    std::vector<std::string> &getStorageTexelBufferOutputs() {
        return storageTexelBufferOutputs_;
    }

    std::vector<std::string> &getStorageImageInputs() {
        return storageImageInputs_;
    }

    std::vector<std::string> &getStorageImageOutputs() {
        return storageImageOutputs_;
    }

    ImageResource &addColorOutput(const std::string &newResource, const AttachmentInfo &info, const std::string &inputName) {
        colorOutputs_.push_back(newResource);
        return pass_->addColorOutput(name_, newResource, info, inputName);
    }

    ImageResource &addAttachmentInput(const std::string &newResource, const AttachmentInfo &info) {
        attachmentInputs_.push_back(newResource);
        return pass_->addAttachmentInput(name_, newResource, info);
    }

    ImageResource &addStorageImageInput(const std::string &newResource, const AttachmentInfo &info) {
        storageImageInputs_.push_back(newResource);
        return pass_->addStorageImageInput(name_, newResource, info);
    }

    ImageResource &addStorageImageOutput(const std::string &newResource, const AttachmentInfo &info, const std::string &inputName = "") {
        storageImageOutputs_.push_back(newResource);
        return pass_->addStorageImageOutput(name_, newResource, info, inputName);
    }

    ImageResource &addDepthStencilInput(const std::string &newResource, const AttachmentInfo &info) {
        assert(depthStencilInput_ == "");
        depthStencilInput_ = newResource;
        return pass_->addDepthStencilInput(name_, newResource, info);
    }

    ImageResource &addDepthStencilOutput(const std::string &newResource, const AttachmentInfo &info) {
        assert(depthStencilOutput_ == "");
        depthStencilOutput_ = newResource;
    }

    ImageResource &addDepthStencilIO(const std::string &newResource, const AttachmentInfo &info) {
        depthStencilInput_ = newResource;
        depthStencilOutput_ = newResource;
    }

    ImageResource &addResolveOutput(const std::string &newResource) {
        resolveOutputs_.push_back(newResource);
    }

    ImageResource &addTextureInput(const std::string &newResource) {
        textureInputs_.push_back(newResource);
    }

    BufferResource &addUniformBuffer(const std::string &newResource) {
        uniformBuffers_.push_back(newResource);
    }

    BufferResource &addUniformTexelBuffer(const std::string &newResource) {
        uniformTexelBuffers_.push_back(newResource);
    }

    BufferResource &addStorageBufferInput(const std::string &newResource) {
        storageBufferInputs_.push_back(newResource);
    }

    BufferResource &addStorageBufferOutput(const std::string &newResource) {
        storageBufferOutputs_.push_back(newResource);
    }

    BufferResource &addStorageTexelBufferInput(const std::string &newResource) {
        storageTexelBufferInputs_.push_back(newResource);
    }

    BufferResource &addStorageTexelBufferOutput(const std::string &newResource) {
        storageTexelBufferOutputs_.push_back(newResource);
    }
    
    // any other adders 

    // SHADER INTERFACE
        // now that we're integrating shaders into everything, i'll have to provide a way to assign
        // shaders into the possible shader stages, which will be used to build the graphics pipeline (per-subpass)
    // now that i think about it, this can be where the actual Graph::Shader objects are created maybe?
        // this doesn't currently check whether the shader module you specify is actually the shader type you want, so 
        // it's on the user for now to be careful
    void registerVertexShader(const std::string &shaderName) {
        vertexShader_ = std::make_shared<Shader>(name_, shaderName, graph_);
    }

    void registerTessellationEvalShader(const std::string &shaderName) {
        tessellationEvalShader_ = std::make_shared<Shader>(name_, shaderName, graph_);
    }

    void registerTessellationControlShader(const std::string &shaderName) {
        tessellationControlShader_ = std::make_shared<Shader>(name_, shaderName, graph_);
    }

    void registerGeometryShader(const std::string &shaderName)  {
        geometryShader_ = std::make_shared<Shader>(name_, shaderName, graph_);
    }

    void registerFragmentShader(const std::string &shaderName) {
        fragmentShader_ = std::make_shared<Shader>(name_, shaderName, graph_);
    }

    std::shared_ptr<Shader> &getVertexShader() {
        return vertexShader_;
    }

    std::shared_ptr<Shader> &getTessellationEvalShader() {
        return tessellationEvalShader_;
    }

    std::shared_ptr<Shader> &getTessellationControlShader() {
        return tessellationControlShader_;
    }

    std::shared_ptr<Shader> &getGeometryShader() {
        return geometryShader_;
    }

    std::shared_ptr<Shader> &getFragmentShader() {
        return fragmentShader_;
    }

    // execution stuff - TODO
    // void execute();

  private:
    /* APR25 - Subpass State */
    std::string name_;
    std::shared_ptr<RenderGraph> graph_ = nullptr;
    std::shared_ptr<ThreadPool> pool_ = nullptr;
    std::shared_ptr<Pass> pass_ = nullptr;
    
    // JobQueue queue_; // probably don't need this

    // for each subpass I think it's probably best to stick the shaders in a few single-capacity slots
    std::shared_ptr<Shader> vertexShader_ = nullptr;
    std::shared_ptr<Shader> tessellationEvalShader_ = nullptr;
    std::shared_ptr<Shader> tessellationControlShader_ = nullptr;
    std::shared_ptr<Shader> geometryShader_ = nullptr;
    std::shared_ptr<Shader> fragmentShader_ = nullptr;

    /* FRAMEBUFFER RESOURCES */
    std::vector<std::string> colorInputs_ = {};
    std::vector<std::string> colorOutputs_ = {};

    std::string depthStencilInput_ = "";
    std::string depthStencilOutput_ = "";

    std::vector<std::string> attachmentInputs_ = {};
    std::vector<std::string> resolveOutputs_ = {};

    std::vector<std::string> storageImageOutputs_ = {};
    std::vector<std::string> storageImageInputs_ = {};

    /* DESCRIPTOR SET RESOURCES */
    // IMPT
        // forgot about the fact that we bind different descriptor sets at different frequencies
            // DS (index) 0 -> GLOBAL RESOURCES: put shader resources here that are bound once per frame, 
                // and make sure that the correct bindings within that set are specified in the render graph
            // DS 1 -> these are for per-PASS resources, which should be bound once per render pass (so
                // they won't be bound every subpass)
            // DS 2 -> "per-material", or otherwise a descriptor set that may be bound at a certain well-defined point
                // in subpass execution; (i think these are pretty much resources that are bound per-subpass)
            // DS 3 -> "per-object" resources, which will be bound most frequently and should contain things that
                // will need to be bound often (not entirely sure how these should fit in yet)
        // it might be possible to just have 4 slots for binding descriptor sets, and always bind at that point in
            // the rendering process... 

    // texture inputs are used with samplers and are bound as part of descriptor sets i think
    std::vector<std::string> textureInputs_ = {};

    // uniforms
    std::vector<std::string> uniformBuffers_ = {};
    std::vector<std::string> uniformTexelBuffers_ = {};

    // storage buffers
    std::vector<std::string> storageBufferInputs_ = {};
    std::vector<std::string> storageBufferOutputs_ = {};

    // storage texel buffers
    std::vector<std::string> storageTexelBufferInputs_ = {};
    std::vector<std::string> storageTexelBufferOutputs_ = {};

    // job queue callback
    // std::function<JobQueue()> buildJobQueue_; // this should build (if necessary) and return a job queue that can be flushed by a thread pool
    
    // any other callbacks...


};