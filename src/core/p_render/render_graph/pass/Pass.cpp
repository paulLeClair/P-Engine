#include "../../../../../include/core/p_render/render_graph/pass/Pass.hpp"

#include "../../../../../include/core/p_render/render_graph/RenderGraph.hpp"

#include "../../../../../include/core/p_render/render_graph/pass/Subpass.hpp"
#include "../../../../../include/core/p_render/render_graph/resources/BufferResource.hpp"
#include "../../../../../include/core/p_render/render_graph/resources/ImageResource.hpp"

// APR25 - rewrite
Pass::Pass(const std::string &name, unsigned int index, std::shared_ptr<ThreadPool> pool, std::shared_ptr<RenderGraph> graph, ResourceQueueUsageFlags usages) 
                        : name_(name), index_(index), pool_(pool), graph_(graph), queueUsages_(usages) {
    
}

const std::string &Pass::getName() const {
    return name_;
}

const unsigned int &Pass::getIndex() const {
    return index_;
}

bool Pass::validate() const {
    // TODO: fill this in
    
    // check input/output sizes; 
        // resources shouldn't be gobbled up by any one pass; each pass must output any resource it takes in
    
    // check that input resource dimensions are the same as output dimensions when relevant

    // any other pass validation checks :)

    // if nothing has been deemed incorrect, return true
    return true;

}

void Pass::bake() {
    // hmmm.. not sure if i need a Pass::bake(), but maybe it could be useful for 
    // building the physical pass info
        // i'm hoping i can just have a Pass::PhysicalPass struct that gets set up for each pass, 
        // maybe bake() can build that so that the bake()d render graph can execute

    // actually i think this works for now


}

void Pass::execute() {
    // BACKEND REWRITE -> no longer use these high level passes in the execution process

    // i think 
}

void Pass::reset() {

}

std::shared_ptr<Subpass> Pass::addSubpass(const std::string &subpassName) {
    // i think it should be important that subpasses are to be executed in the order they're defined
        // it shouldn't really matter when you add your resource information since you have to specify it per-subpass
    auto self = shared_from_this();
    auto subpass = std::make_shared<Subpass>(subpassName, graph_, pool_, self);
    subpasses_.push_back(subpass);
    subpassNames_[subpassName] = subpasses_.size() - 1;
    return subpass;
}

ImageResource &Pass::addColorOutput(const std::string &subpassName, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName) {
    // get or create an image resource for the color output
    auto &resource = graph_->getImageResource(outputName, &attachmentInfo);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addWritePass(index_);
    // resource.setAttachmentInfo(attachmentInfo);
    resource.addImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    if (attachmentInfo.numLevels != 1) {
        // handle this somehow...
    }

    // push back the new color output
    colorOutputs_.push_back(&resource);

    // the subpass sets up its own stuff now

    // add corresponding input
    if (!inputName.empty()) {
        // if an input name is given, get or create the image input resource
        auto &inputResource = graph_->getImageResource(inputName);
        inputResource.addReadPass(index_);
        inputResource.addImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

        // add the new color input 
        colorInputs_.push_back(&inputResource);
    }
    else {
        // if no input is given, i guess we do actually push an empty input...
        colorInputs_.push_back(nullptr);
        // color scale inputs might have to be handled here too?
    }

    return resource;
}

ImageResource &Pass::addDepthStencilInput(const std::string &subpassName, const std::string &inputName, const AttachmentInfo &attachmentInfo) {
    // get or create the resource
    auto &resource = graph_->getImageResource(inputName);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addReadPass(index_);
    resource.addImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // set the actual depth stencil slot in this pass
    depthStencilInput_ = &resource;

    return resource;
}

ImageResource &Pass::addDepthStencilOutput(const std::string &subpassName, const std::string &outputName, const AttachmentInfo &outputAttachmentInfo) {
    // get or create the resource
    auto &resource = graph_->getImageResource(outputName, &outputAttachmentInfo);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addWritePass(index_);
    // resource.setAttachmentInfo(outputAttachmentInfo);
    resource.addImageUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthStencilOutput_ = &resource;

    return resource;
}

ImageResource &Pass::addAttachmentInput(const std::string &subpassName, const std::string &name, const AttachmentInfo &attachmentInfo) {
    // get or create the resource
    auto &resource = graph_->getImageResource(name);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addReadPass(index_);
    resource.addImageUsage(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);

    attachmentInputs_.push_back(&resource);
    return resource;
}

ImageResource &Pass::addStorageImageOutput(const std::string &subpassName, const std::string &outputName, const AttachmentInfo &attachmentInfo, const std::string &inputName) {
    auto &resource = graph_->getImageResource(outputName);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addWritePass(index_);
    // resource.setAttachmentInfo(attachmentInfo);
    resource.addImageUsage(VK_IMAGE_USAGE_STORAGE_BIT);
    storageImageOutputs_.push_back(&resource);

    if (!inputName.empty()) {
        // get/create input resource
        auto &input = graph_->getImageResource(inputName);
        input.addReadPass(index_);
        input.addImageUsage(VK_IMAGE_USAGE_STORAGE_BIT);
        storageImageInputs_.push_back(&input);
    }
    else {
        storageImageInputs_.push_back(nullptr);
    }

    return resource;
}

BufferResource &Pass::addStorageBufferOutput(const std::string &subpassName, const std::string &outputName, const BufferInfo &bufferInfo, const std::string &inputName) {
    auto &resource = graph_->getBufferResource(outputName);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addWritePass(index_);
    resource.addBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    storageBufferOutputs_.push_back(&resource);

    if (!inputName.empty()) {
        auto &input = graph_->getBufferResource(inputName);
        input.addReadPass(index_);
        input.addBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        storageBufferInputs_.push_back(&input);
    }
    else {
        storageBufferInputs_.push_back(nullptr);
    }

    return resource;
}

// BufferResource &Pass::addStorageBufferInput(const std::string &subpassName, const std::string &name, const BufferInfo &info) {
//     // this makes use of generic buffers
//     // if (stages == 0) {
//     //     // set it 
//     //     stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     // }

//     // it might not work to be able to add storage buffer inputs like this... 
//         // maybe for input/output type resources, the input can only be added in conjunction with an output, so there are no isolated inputs 
//         // for things like that 
//     // kinda makes sense; a read-only storage buffer i believe might as well just be a uniform buffer, so maybe it would be better to just
//     // enforce that the user configures it that way 

//     // return addGenericBufferInput(name, stages, VK_ACCESS_SHADER_READ_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//     auto &resource = graph_->getBufferResource(name);
//     resource.addResourceQueueUsage(queueUsages_); // not sure how useful this mechanism is currently
//     resource.addReadPass(index_);
//     resource.addBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//     storageBufferInputs_.push_back(&resource);

//     return resource; // hopefully that does it

// }

// BufferResource &Pass::addVertexBuffer(const std::string &name) {
//     return addGenericBufferInput(name, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
// }

// BufferResource &Pass::addIndexBuffer(const std::string &name) {
//     return addGenericBufferInput(name, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
// }

BufferResource &Pass::addUniformBufferInput(const std::string &subpassName, const std::string &name, const BufferInfo &info) {
    // if (stages == 0) {
    //     stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    // }

    // return addGenericBufferInput(name, stages, VK_ACCESS_UNIFORM_READ_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    auto &resource = graph_->getBufferResource(name);
    resource.addResourceQueueUsage(queueUsages_);
    resource.addReadPass(index_);
    resource.addBufferUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    uniformBuffers_.push_back(&resource);

    return resource;
}

BufferResource &Pass::addUniformTexelBufferInput(const std::string &subpassName, const std::string &name, const BufferInfo &info) {
    // hopefully this works
    auto &resource = graph_->getBufferResource(name);
    resource.addResourceQueueUsage(queueUsages_); 
    resource.addReadPass(index_);
    resource.addBufferUsage(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    uniformTexelBuffers_.push_back(&resource);

    // leaving this one :)
    // names courtesy of Melanie MacPhail in honor of the Computer Gods
    // auto &doIt = subpassNames_[subpassName];
    // auto &please = subpasses_[doIt];
    // please->addUniformTexelBuffer(name);

    return resource;
}