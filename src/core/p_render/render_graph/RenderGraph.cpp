#include "../../../../include/core/p_render/render_graph/RenderGraph.hpp"

#include "../../../../include/core/p_render/render_graph/pass/Pass.hpp"
#include "../../../../include/core/p_render/backend/PhysicalPass.hpp"

#include "../../../../include/core/p_render/backend/frame_context/FrameContext.hpp"

#include "../../../../include/core/p_render/render_graph/resources/SwapchainImageResource.hpp"
#include "../../../../include/core/p_render/render_graph/resources/ImageResource.hpp"
#include "../../../../include/core/p_render/render_graph/resources/BufferResource.hpp"
#include "../../../../include/core/p_render/render_graph/resources/ResourceDimensions.hpp"

#include "../../../../include/core/p_render/backend/resources/Image.hpp"
#include "../../../../include/core/p_render/backend/resources/SwapchainImage.hpp"
#include "../../../../include/core/p_render/backend/resources/Buffer.hpp"

#include "../../../../include/core/p_render/backend/shaders/ShaderModule.hpp"


RenderGraph::RenderGraph(const std::string &name, std::shared_ptr<ThreadPool> pool, std::shared_ptr<Backend::Context> context, VmaAllocator allocator) : name_(name), pool_(std::move(pool)), context_(std::move(context)), allocator_(allocator) {

    auto swapchainDim = getSwapchainDimensions();
    swapchainDimensions_ = swapchainDim; // store this separately (kinda ugly)
    AttachmentInfo swapchainInfo = {};
    swapchainInfo.format = swapchainDim->format;
    swapchainInfo.numLayers = swapchainDim->layers;
    swapchainInfo.numLevels = swapchainDim->mipLevels;
    swapchainInfo.numSamples = swapchainDim->samples;
    
    swapchainInfo.persistent = true;
    swapchainInfo.transient = false;

    swapchainInfo.sizeClass = ImageSizeClass::SwapchainRelative;
    
    swapchainInfo.size_x = swapchainDim->width; 
    swapchainInfo.size_y = swapchainDim->height;
    swapchainInfo.size_z = swapchainDim->depth;

    auto swapchainImageResource = std::make_shared<SwapchainImageResource>(0, "swapchain", swapchainInfo, context_);

    resources_.push_back(swapchainImageResource);
    resourceNames_["swapchain"] = resources_.size() - 1;

    // we'll default the backbuffer source to this for now...
    backbufferResourceName_ = "swapchain";
}

RenderGraph::~RenderGraph() {
    // TODO
}

/* RENDER GRAPH INTERFACE */
#pragma region INTERFACE_RG

void RenderGraph::bake() {
    // APR26 - this is the massive baking function for preparing a render graph for execution 

    // first make sure there's a backbuffer resource we can use
    auto itr = resourceNames_.find(backbufferResourceName_);
    if (itr == resourceNames_.end() || backbufferResourceName_ == "") {
        throw std::runtime_error("No backbuffer source has been specified!");
    }

    if (resources_[resourceNames_[backbufferResourceName_]]->getWritePasses().size() == 0) {
        throw std::runtime_error("No passes write to the backbuffer!");
    }

    validatePasses(); // this must succeed before we can continue

    // set up the pass stack
    const auto setupPassStack = [&]() {
        passStack_.clear();
        passDependencies_.clear();
        passMergeDependencies_.clear();

        passDependencies_.resize(passes_.size());
        passMergeDependencies_.resize(passes_.size());

        analyzeDependencies();

        // after analyzing dependencies, we have to reverse the order of the stack and filter them
        std::reverse(passStack_.begin(), passStack_.end());
        filterPasses();

        reorderPasses(); // could make this heuristic reordering step optional; might be a good idea if any issues arise

    };
    setupPassStack();

    /* BUILD PHYSICAL RESOURCES + RMW ALIASES */

    buildPhysicalResources();
    
    
    /* BUILD PHYSICAL PASSES */
    buildPhysicalPasses();

    // need to make sure every physical pass has all the proper barriers set up,
        // need to do more research to make sure i handle the important cases
    buildBarriers();

    buildAliases(); // this refers specifically to TEMPORAL aliasing of images, which allows us to reuse non-overlapping compatible VkImages
    // and that could be helpful when we have large graphs with lots of images

    baked_ = true;
}

void RenderGraph::execute(Backend::FrameContext &frame) {
    // whatever pre-execute setup is required?
    
    // handle swapchain
    const auto setupSwapchainImageResourceForFrame = [&](unsigned int index) {
        // auto defaultSwapchainImage = std::dynamic_pointer_cast<SwapchainImageResource>(resources_[resourceNames_["swapchain"]]);
        // defaultSwapchainImage->setActiveSwapchainImageIndex(index); // this should do for now i guess.. 
        auto physIndex = resources_[resourceNames_["swapchain"]]->getPhysicalIndex();
        auto scImg = std::dynamic_pointer_cast<Backend::SwapchainImage>(physicalResources_[physIndex]);
        scImg->setActiveSwapchainIndex(index);
    };

    setupSwapchainImageResourceForFrame(frame.getIndex());

    if (!baked_) bake(); 
    
    // this should actually execute the flattened array of physical passes, using what was built up in the bake() 
    // i may have to flesh this out a bit more 
    for (auto &physicalPass : physicalPasses_) {
        physicalPass->execute();
    }
}

// not sure if this function is deprecated
void RenderGraph::reset() {
    for (auto &pass : passes_) {
        pass->reset();
    }

}

Pass &RenderGraph::getPass(const std::string &name) {
    // first declare a lambda function that encodes our search criterion
    const auto finderFunction = [&name](const std::shared_ptr<Pass> &pass) {
        return pass->getName() == name;
    };

    // use find_if to find the pass if it exists
    const auto find = std::find_if(passes_.begin(), passes_.end(), finderFunction);
    if (find == passes_.end()) {
       // TODO: something here lol
       throw std::runtime_error("Unable to find pass!");
    }
    return **find;
}

// functions for building/editing a render graph
std::shared_ptr<Pass> RenderGraph::appendPass(const std::string &name) {
    if (baked_) {
        // might get rid of this check eventually
        throw std::runtime_error("Attempting to append to a finalized render graph!");
    }

    // make sure that no pass exists with the same name
    for (const auto &existingPass : passes_) {
        if (existingPass->getName() == name) {
            throw std::runtime_error("There is already a pass with name " + name);
        }
    }

    // set up the pass and return it
        // to package things up better i might wanna replace these individual args with a struct 
    auto pass = std::make_shared<Pass>(name, static_cast<unsigned int>(passes_.size()), std::move(pool_), shared_from_this());
    passes_.push_back(pass);
    return pass;
}

#pragma endregion INTERFACE_RG

/* RESOURCES */
#pragma region RESOURCES_RG
BufferResource &RenderGraph::getBufferResource(const std::string &name, const BufferInfo *info) {
    // using the same logic as Themaister here: 
        // if it doesnt exist, create it and return it
        // if it exists, verify it's a buffer and return it 
    auto itr = resourceNames_.find(name);
    if (itr != resourceNames_.end()) {
        assert(resources_[itr->second]->getType() == RenderResource::Type::Buffer);
        return static_cast<BufferResource&>(*resources_[itr->second]);
    }
    else {
        // the resource has to be created
        if (!info) {
            throw std::runtime_error("No info provided to new buffer resource!");
        }

        unsigned int index = resources_.size();
        resources_.emplace_back(std::move(std::make_shared<BufferResource>(index, name, *info)));
        
        // set the resource's name etc
        resourceNames_[name] = index;

        return static_cast<BufferResource&>(*resources_.back());
    }
}

ImageResource &RenderGraph::getImageResource(const std::string &name, const AttachmentInfo *info) {
    auto itr = resourceNames_.find(name);
    if (itr != resourceNames_.end()) {
        assert(resources_[itr->second]->getType() == RenderResource::Type::Image);
        return static_cast<ImageResource&>(*resources_[itr->second]);
    }
    else {
        // resource has to be created 
        if (!info) {
            throw std::runtime_error("No info provided for new image resource!");
        }

        unsigned int index = resources_.size();
        resources_.emplace_back(std::move(std::make_shared<ImageResource>(index, name, *info)));

        resourceNames_[name] = index;

        return static_cast<ImageResource&>(*resources_.back());
    }
}
#pragma endregion RESOURCES_RG

/* SHADERS */
std::shared_ptr<Backend::ShaderModule> RenderGraph::getShaderModule(const std::string &name) {
    // similarly to the functions for images and resources, we will either return an
    // existing resource's handle, or we'll create it and then return it
    if (shaderModuleNames_.find(name) != shaderModuleNames_.end()) {
        return shaderModules_[shaderModuleNames_[name]];
    }
    
    shaderModules_.push_back(std::move(std::make_shared<Backend::ShaderModule>(name, context_, shared_from_this())));
    shaderModuleNames_[name] = shaderModules_.size() - 1; // set name->index mapping for this module
    return shaderModules_[shaderModuleNames_[name]];
}

/* BAKING UTITILITES */
#pragma region BAKE_RG
void RenderGraph::validatePasses() {
    for (const auto &pass : passes_) {
        if (!pass->validate()) {
            // this shouldn't crash the program but for now i'll just catch it with a runtime error as is the temporary fix
            throw std::runtime_error("Pass " + pass->getName() + " is invalid!");
        }
    }
}

void RenderGraph::analyzeDependencies() {
    // traverse dependencies, starting from the backbuffer resource
    auto &backbufferResource = *resources_[resourceNames_[backbufferResourceName_]];

    // first, push all passes that write the backbuffer to the pass stack 
    assert(passStack_.size() == 0);
    for (auto &backbufferWritePass : backbufferResource.getWritePasses()) {
        passStack_.push_back(backbufferWritePass);
    }

    // now traverse all these original passes in the pass stack
        // that's why we create the temp stack at this point
    auto &tempPassStack = passStack_;
    for (auto &passIndex : tempPassStack) {
        auto &pass = *passes_[passIndex];
        traverseDependencies(pass, 0); // traverse each pass recursively
    }
}

void RenderGraph::traverseDependencies(Pass &pass, unsigned int stackCounter) {

    // here we go through each resource type and recursively check their dependencies

    if (pass.getDepthStencilInput()) {
        recursePassDependencies(pass, pass.getDepthStencilInput()->getWritePasses(), stackCounter, false, false, true);
    }

    for (auto *input : pass.getColorInputs()) {
        if (input)
            recursePassDependencies(pass, input->getWritePasses(), stackCounter, false, false, true);
    }

    for (auto *input : pass.getStorageImageInputs()) {
        if (input) {
            recursePassDependencies(pass, input->getWritePasses(), stackCounter, true, false, false);
            recursePassDependencies(pass, input->getReadPasses(), stackCounter, true, true, false);
        }
    }

    for (auto *input : pass.getTextureInputs()) {
        if (input)
            recursePassDependencies(pass, input->getWritePasses(), stackCounter, false, false, false);
    }

    for (auto *input : pass.getUniformBuffers()) {
        if (input)
            recursePassDependencies(pass, input->getWritePasses(), stackCounter, false, false, false);
    }

    for (auto *input : pass.getUniformTexelBuffers()) {
        if (input) 
            recursePassDependencies(pass, input->getWritePasses(), stackCounter, false, false, false);
    }

}

void RenderGraph::recursePassDependencies(const Pass &self, std::unordered_set<unsigned int> &writtenPasses, unsigned int stackCounter, bool noCheck, bool ignoreSelf, bool mergeDependency) {

    // first filter our recursion based on the given flags
    if (!noCheck && writtenPasses.empty())
        throw std::runtime_error("No pass exists which writes to the resource!");

    if (stackCounter > passes_.size()) {
        throw std::runtime_error("Cycle detected, or some other issue with specified graph!");
    }

    if (mergeDependency) {
        for (auto &pass : writtenPasses) {
            if (pass != self.getIndex()) {
                // found a merge dependency!
                passMergeDependencies_[self.getIndex()].insert(pass);
            }
        }
    }

    // increment stack count
    stackCounter++;

    for (auto writePass : writtenPasses) {
        // checks
        if (ignoreSelf && writePass == self.getIndex()) {
            continue;
        }
        else if (writePass == self.getIndex()) {
            throw std::runtime_error("Pass depends on itself!");
        }

        // push and recursively add dependencies of all passes 
        passStack_.push_back(writePass);
        auto &pass = *passes_[writePass];
        traverseDependencies(pass, stackCounter);
    }
}

bool RenderGraph::dependsOnPass(unsigned int destinationPass, unsigned int sourcePass) {
    if (destinationPass == sourcePass) {
        return true;
    }

    for (auto &dep : passDependencies_[destinationPass]) {
        if (dependsOnPass(dep, sourcePass)) {
            return true;
        }
    }

    return false;
}

void RenderGraph::filterPasses() {
    // i think this should just prune duplicate passes from the graph, but it could do whatever other filtering you want i guess
    std::unordered_set<unsigned int> seenSet;

    auto outputItr = passStack_.begin();
    for (auto itr = passStack_.begin(); itr != passStack_.end(); itr++) {
        if (!seenSet.count(*itr)) {
            *outputItr = *itr; 
            seenSet.insert(*itr);
            outputItr++;
        }
    }
    passStack_.erase(outputItr, passStack_.end());

}

void RenderGraph::reorderPasses() {

    // after determining dependencies we have to reorder the pass stack

    // i think the idea here is to first copy over dependencies to the dependees of passes
    // to prevent some issues with subpass merging later
    for (auto &mergeDependencies : passMergeDependencies_) {
        // first get the index of the pass whose merge dependencies we're looking at 
        auto passIndex = static_cast<unsigned int>(&mergeDependencies - passMergeDependencies_.data()); // use ptr arith to compute index
        // get dependencies (vector of indices of passes that the current pass depends on)
        auto &passDependencies = passDependencies_[passIndex];

        // go over dependencies for this pass
        for (auto &mergeDependency : mergeDependencies) {
            for (auto &dependee : passDependencies) {
                // avoid cycles
                if (dependsOnPass(dependee, mergeDependency))
                    continue;
                
                if (mergeDependency != dependee) {
                    passDependencies_[mergeDependency].insert(dependee);
                }
            }
        }
    }

    // now we actually schedule/organize the passes!
    if (passStack_.size() <= 2) {
        return;
    }

    // empty-swap to clear the passStack_
    std::vector<unsigned int> unscheduledPasses;
    unscheduledPasses.reserve(passes_.size());
    std::swap(passStack_, unscheduledPasses);

    // lambda that will be used for scheduling the actual passes
    const auto schedule = [&](unsigned int index) {
        passStack_.push_back(unscheduledPasses[index]);
        std::move(unscheduledPasses.begin() + index + 1, unscheduledPasses.end(), unscheduledPasses.begin() + index);
        unscheduledPasses.pop_back();
    };

    // begin scheduling with the first pass
    schedule(0);

    // loop over all unscheduled passes
    while (!unscheduledPasses.empty()) {
        // while we're looping, we maintain variables for our best scheduling candidate (plus the metric used)
        unsigned int bestCandidate = 0;
        unsigned int bestOverlapFactor = 0;

        // look through all remaining unscheduled passes
        for (unsigned int i = 0; i < unscheduledPasses.size(); i++) {
            // for each pass, maintain an overlap factor (scheduling metric)
            unsigned int overlapFactor = 0;

            if (passMergeDependencies_[unscheduledPasses[i]].count(passStack_.back())) {
                // in this case, we set the score heuristic to max to ensure this pass should be scheduled next 
                // (since it can merge with the last scheduled pass)
                overlapFactor = ~0u;
            }
            else {
                // for the given unscheduled pass, go through the stack in reverse order until a dependency is found;
                // the number of passes in the stack until the next dependency is the overlap factor for the current unscheduled pass
                for (auto itr = passStack_.rbegin(); itr != passStack_.rend(); itr++) {
                    if (dependsOnPass(unscheduledPasses[i], *itr)) {
                        break;
                    }
                    overlapFactor++;
                }
            }
            // after overlap factor has been computed, compare it to the best overlap factor we've seen so far
            if (overlapFactor <= bestOverlapFactor) 
                continue;

            // in this case we may have a new best candidate, which we'll mark with a boolean
            bool possibleCandidate = true;

            // however, a pass might not be a possible candidate if it depends on any earlier unscheduled passes 
            for (unsigned int j = 0; j < i; j++) {
                if (dependsOnPass(unscheduledPasses[i], unscheduledPasses[j])) {
                    possibleCandidate = false;
                    break;
                }
            }

            // check result of test
            if (!possibleCandidate) 
                continue;

            // if it passed the test, then we have a new best candidate for scheduling
            bestCandidate = i; 
            bestOverlapFactor = overlapFactor;
        }

        // after we've passed each of the unscheduled passes, schedule the best one we've found
        schedule(bestCandidate);
    }
}

// get or create resource dimensions given render graph buffer or images
ResourceDimensions RenderGraph::getResourceDimensions(BufferResource &resource) const {
    // build ResourceDimensions for the given resource
    ResourceDimensions dim; 
    auto &info = resource.getBufferInfo();
    dim.bufferInfo = info;
    dim.bufferInfo.usage |= resource.getBufferUsages();
    
    // could get flags like persistence etc here but they do nothing currently

    dim.name = resource.getResourceName();

    return dim;
}

ResourceDimensions RenderGraph::getResourceDimensions(ImageResource &resource) const {
    ResourceDimensions dim;
    auto &info = resource.getAttachmentInfo();

    // get relevant image data for creating the vulkan image
    dim.layers = info.numLayers;
    dim.samples = info.numSamples;
    dim.format = info.format;
    
    // various extra fancy flags 
    dim.transient = info.transient;
    dim.persistent = info.persistent;

    // could add more here i blieve

    dim.queueUsages = resource.getResourceQueueUsages();
    dim.imageUsage = info.auxUsage | resource.getImageUsageFlags();
    dim.name = resource.getResourceName();

    // now we set up the spatial dimensions of the image depending on the chosen size class
    assert(swapchainDimensions_->width && swapchainDimensions_->height); // jjust wanna make sure
    // that swapchain dimensions exist at this point 

    switch (info.sizeClass) {
        case (ImageSizeClass::SwapchainRelative):
            // i think this is the most common case
            dim.width = std::max(unsigned int(std::ceilf(info.size_x * swapchainDimensions_->width)), 1u);
            dim.height = std::max(unsigned int(std::ceilf(info.size_y * swapchainDimensions_->height)), 1u);
            dim.depth = std::max(unsigned int(std::ceilf(info.size_z)), 1u);
        break;

        case (ImageSizeClass::InputRelative): {
            // this size class will make use of a particular resource name to size the image

            // find the input image to size
            auto itr = resourceNames_.find(info.sizeRelativeName);
            if (itr == resourceNames_.end()) {
                throw std::runtime_error("Relative sizing resource not found!");
            }
            
            // if it was found, acquire the dimensions
            auto &input = static_cast<ImageResource&>(*resources_[itr->second]);
            auto inputDim = getResourceDimensions(input);

            dim.width = std::max(unsigned int(std::ceilf(info.size_x * inputDim.width)), 1u);
            dim.height = std::max(unsigned int(std::ceilf(info.size_y * inputDim.height)), 1u);
            dim.depth = std::max(unsigned int(std::ceilf(info.size_z * inputDim.depth)), 1u);
            break;
        }
        case (ImageSizeClass::Absolute):
            // this just absolutely sets the size
            dim.width = std::max(unsigned int(info.size_x), 1u);
            dim.height = std::max(unsigned int(info.size_y), 1u);
            dim.depth = std::max(unsigned int(info.size_x), 1u);
        break;
    }

    // set format if needed
    if (dim.format == VK_FORMAT_UNDEFINED) {
        dim.format = swapchainDimensions_->format;
    }

    // TODO: compute number of mip levels here (it would be nice to have automatic mip-map generation at some point)
    
    return dim;
}

void RenderGraph::buildPhysicalResources() {
    // after the passes have been flattened and ordered, we can build the physical resources corresponding to the
    // high-level graph resources that were specified 

    // initialize a variable corresponding to the current index in the physical resources array (which should start from empty)
    unsigned int physicalResourceIndex = 0;
    physicalResourceDimensions_.clear();

    // here we build up the physicalResourceDimensions_ vector with an entry for each of the physical resources used by each PASS
    for (auto &passIndex : passStack_) {
        // get the pass
        auto &pass = *passes_[passIndex];

        // now go through each resource type in the pass and build+alias resources

        // TODO: handle everything that was previously done as a generic texture/buffer

        // storage inputs
        if (!pass.getStorageBufferInputs().empty()) {
            for (auto *storageInput : pass.getStorageBufferInputs()) { 
                if (storageInput->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*storageInput);
                    physicalResourceDimensions_.push_back(dim);

                    storageInput->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[storageInput->getPhysicalIndex()].queueUsages |= storageInput->getResourceQueueUsages();
                    physicalResourceDimensions_[storageInput->getPhysicalIndex()].bufferInfo.usage |= storageInput->getBufferUsages();
                }
            }
        }

        // storage texture inputs 
        if (!pass.getStorageImageInputs().empty()) {
            for (auto *storageTextureInput : pass.getStorageImageInputs()) {
                
                if (storageTextureInput->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*storageTextureInput);
                    physicalResourceDimensions_.push_back(dim);

                    storageTextureInput->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[storageTextureInput->getPhysicalIndex()].queueUsages |= storageTextureInput->getResourceQueueUsages();
                    physicalResourceDimensions_[storageTextureInput->getPhysicalIndex()].imageUsage |= storageTextureInput->getImageUsageFlags();
                }
            }
        }

        // color inputs go last, so they can alias properly with existing color+depth attachments in the same subpass
        if (!pass.getColorInputs().empty()) {
            unsigned int size = pass.getColorInputs().size();
            for (unsigned int i = 0; i < size; i++) {
                auto *input = pass.getColorInputs()[i]; 
                if (input) {
                    if (input->getPhysicalIndex() == ~0u) {
                        auto &dim = getResourceDimensions(*input);
                        physicalResourceDimensions_.push_back(dim);

                        input->setPhysicalIndex(physicalResourceIndex++);
                    }
                    else {
                        physicalResourceDimensions_[input->getPhysicalIndex()].queueUsages |= input->getResourceQueueUsages();
                        physicalResourceDimensions_[input->getPhysicalIndex()].imageUsage |= input->getImageUsageFlags();
                    }

                    // check corresponding color output; we need to alias Read-Modify-Write resources in each pass
                    if (pass.getColorOutputs()[i]->getPhysicalIndex() == ~0u) {
                        // set the physical index if it hasn't been set
                        pass.getColorOutputs()[i]->setPhysicalIndex(input->getPhysicalIndex());
                    }
                    else if (pass.getColorOutputs()[i]->getPhysicalIndex() != input->getPhysicalIndex()) {
                        throw std::runtime_error("Cannot alias resources for some reason!!!");
                    }
                }
            }
        }
        
        // depth/stencil input + output
        auto *dsInput = pass.getDepthStencilInput();
        if (dsInput) {
            if (dsInput->getPhysicalIndex() == ~0u) {
                auto &dim = getResourceDimensions(*dsInput);
                physicalResourceDimensions_.push_back(dim);

                dsInput->setPhysicalIndex(physicalResourceIndex++);
            }
            else {
                physicalResourceDimensions_[dsInput->getPhysicalIndex()].queueUsages |= dsInput->getResourceQueueUsages();
                physicalResourceDimensions_[dsInput->getPhysicalIndex()].imageUsage |= dsInput->getImageUsageFlags();
            }
        }
        
        auto *dsOutput = pass.getDepthStencilOutput();
        if (dsOutput) {
            if (dsOutput->getPhysicalIndex() == ~0u) {
                auto &dim = getResourceDimensions(*dsOutput);
                physicalResourceDimensions_.push_back(dim);

                dsOutput->setPhysicalIndex(physicalResourceIndex++);
            }
            else {
                physicalResourceDimensions_[dsOutput->getPhysicalIndex()].queueUsages |= dsOutput->getResourceQueueUsages();
                physicalResourceDimensions_[dsOutput->getPhysicalIndex()].imageUsage |= dsOutput->getImageUsageFlags();
            }
        }

        // now onto the rest of the pass' outputs!

        // storage buffer + texture outputs
        if (!pass.getStorageBufferOutputs().empty()) {
            for (auto *output : pass.getStorageBufferOutputs()) {
                if (output->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*output);
                    physicalResourceDimensions_.push_back(dim);

                    output->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[output->getPhysicalIndex()].queueUsages |= output->getResourceQueueUsages();
                    physicalResourceDimensions_[output->getPhysicalIndex()].imageUsage |= output->getBufferUsages();
                }
            }
        }

        if (!pass.getStorageImageOutputs().empty()) {
            for (auto *output : pass.getStorageImageOutputs()) {
                if (output->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*output);
                    physicalResourceDimensions_.push_back(dim);

                    output->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[output->getPhysicalIndex()].queueUsages |= output->getResourceQueueUsages();
                    physicalResourceDimensions_[output->getPhysicalIndex()].imageUsage |= output->getImageUsageFlags();
                }
            }
        }

        if (!pass.getResolveOutputs().empty()) {
            for (auto *output : pass.getResolveOutputs()) {
                if (output->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*output);
                    physicalResourceDimensions_.push_back(dim);

                    output->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[output->getPhysicalIndex()].queueUsages |= output->getResourceQueueUsages();
                    physicalResourceDimensions_[output->getPhysicalIndex()].imageUsage |= output->getImageUsageFlags();
                }
            }
        }

        // color
        if (!pass.getColorOutputs().empty()) {
            for (auto *output : pass.getColorOutputs()) {
                if (output->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*output);
                    physicalResourceDimensions_.push_back(dim);

                    output->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[output->getPhysicalIndex()].queueUsages |= output->getResourceQueueUsages();
                    physicalResourceDimensions_[output->getPhysicalIndex()].imageUsage |= output->getImageUsageFlags();
                }
            }
        }
        
        // input attachments should go last so they can alias properly with existing depth+color attachments in the same subpass
        if (!pass.getAttachmentInputs().empty()) {
            for (auto *input : pass.getAttachmentInputs()) {
                if (input->getPhysicalIndex() == ~0u) {
                    auto &dim = getResourceDimensions(*input);
                    physicalResourceDimensions_.push_back(dim);

                    input->setPhysicalIndex(physicalResourceIndex++);
                }
                else {
                    physicalResourceDimensions_[input->getPhysicalIndex()].queueUsages |= input->getResourceQueueUsages();
                    physicalResourceDimensions_[input->getPhysicalIndex()].imageUsage |= input->getImageUsageFlags();
                }
            }
        }
    }

    // at this point, i should probably use these physicalResourceDimensions_ to build actual resources...
    physicalResources_.clear();
    for (auto i = 0u; i < physicalResourceDimensions_.size(); i++) {
        auto &dim = physicalResourceDimensions_[i];

        if (dim.bufferInfo.size) {
            // make a buffer!
            Backend::Buffer::BufferCreateInfo createInfo = {};

            // first fill any additional info...
            createInfo.allocator = allocator_;
            

            createInfo.size = dim.bufferInfo.size;
            createInfo.usages = dim.bufferInfo.usage;

            // donno exactly where to set the domain, but should add that in somewhere so that
            // other memory usages work

            physicalResources_.push_back(std::make_shared<Backend::Buffer>(createInfo));
        }
        else {
            // handle swapchain images separately
            if (dim.name == "swapchain") {
                std::vector<VkImage> swapchainImages = {};
                for (int j = 0; j < context_->getSwapchainImageCount(); j++) {
                    swapchainImages.push_back(context_->getSwapchainImage(j));
                }
                auto scImg = std::make_shared<Backend::SwapchainImage>(swapchainImages);

                physicalResources_.push_back(scImg);
                continue;
            }
            
            // make an image based off the ResourceDimensions!
            Backend::Image::ImageCreateInfo createInfo = {};

            createInfo.allocator = allocator_;
            createInfo.context = context_;

            createInfo.width = dim.width;
            createInfo.height = dim.height;
            createInfo.depth = dim.depth;

            createInfo.format = dim.format; // forgot this i think

            // should probably use a different layout depending on the 
            // type/usage of the image... but it's cool for now 
            createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            
            switch (dim.samples) {
                case (2):
                    createInfo.samples = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case (4):
                    createInfo.samples = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case (8):
                    createInfo.samples = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case (16):
                    createInfo.samples = VK_SAMPLE_COUNT_16_BIT;
                    break;
                default:
                    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;    
            }

            createInfo.layers = 1;
            createInfo.levels = 1;

            physicalResources_.push_back(std::make_shared<Backend::Image>(createInfo));
        }
    }

    // hopefully at this point, all the physical resources should exist...

}

void RenderGraph::buildPhysicalPasses() {
    // now i think we go on to building actual physical passes!

    // first we define some handy lambdas that will be used to merge physical passes
        // we'll make one PhysicalPass object for each pass and merge them using these lambdas

    // yee
    const auto find_attachment = [](const std::vector<ImageResource*> &resourceList, const ImageResource *resource) -> bool {
        if (!resource)
            return false;
        
        auto itr = std::find_if(resourceList.begin(), resourceList.end(), [resource](const ImageResource *res) {
            return res->getPhysicalIndex() == resource->getPhysicalIndex();
        });
        return itr != resourceList.end();
    };

    // frigge
    const auto find_buffer = [](const std::vector<BufferResource*> &resourceList, const BufferResource *resource) {
        if (!resource)
            return false;
        
        auto itr = std::find_if(resourceList.begin(), resourceList.end(), [resource](const BufferResource *res) {
            return res->getPhysicalIndex() == resource->getPhysicalIndex();
        });

        return itr != resourceList.end();
    };

    const auto should_merge = [&find_attachment, &find_buffer](Pass &prev, Pass &next) -> bool {
        // here we include tests for whether we can merge a pass...
            // these can change with time but for now i'm just gonna kinda keep it simple
        
        // BEGIN FAILURE CASES (where we shouldn't merge, which we check before looking for conditions where we should merge)

        // first test: check for mip map generation (we can't merge if we're generating later)
        for (auto *output : prev.getColorOutputs()) {
            
        }

        // check for non-local dependencies
            // here we're looping over generic textures but i'm not using those so i guess ill just go over normal textures?
        for (auto &input : next.getTextureInputs()) {
            if (find_attachment(prev.getColorInputs(), input)) {
                return false;
            }
            if (find_attachment(prev.getResolveOutputs(), input)) {
                return false;
            }
            if (find_attachment(prev.getStorageImageInputs(), input)) {
                return false;
            }
            // TODO: blit outputs
            if (input  && prev.getDepthStencilOutput() == input) {
                return false;
            }
        }

        // TODO: check blit texture inputs

        for (auto &input : next.getStorageBufferInputs()) {
            if (find_buffer(prev.getStorageBufferOutputs(), input)) {
                return false;
            }
        }

        for (auto &input : next.getStorageImageInputs()) {
            if (find_attachment(prev.getStorageImageOutputs(), input)) {
                return false;
            }
        }

        // TODO: check color scale inputs here

        // some temp lambdas:
        const auto different_attachment = [](const RenderResource *a, const RenderResource *b) {
            return a && b && a->getPhysicalIndex() != b->getPhysicalIndex();
        };

        const auto same_attachment = [](const RenderResource *a, const RenderResource *b) {
            return a && b && a->getPhysicalIndex() == b->getPhysicalIndex();
        };
        
        // check depth attachments
        if (different_attachment(next.getDepthStencilInput(), prev.getDepthStencilInput()))
            return false;
        
        if (different_attachment(next.getDepthStencilOutput(), prev.getDepthStencilInput()))
            return false;
        
        if (different_attachment(next.getDepthStencilInput(), prev.getDepthStencilOutput()))
            return false;
        
        if (different_attachment(next.getDepthStencilOutput(), prev.getDepthStencilOutput()))
            return false;

        for (auto *input : next.getColorInputs()) {
            if (!input)
                continue;
            
            if (find_attachment(prev.getStorageImageOutputs(), input))
                return false;
            // todo: check blit texture outputs
        }

        // END FAILURE CASES

        // BEGIN SUCCESS CASES
        for (auto *input : next.getColorInputs()) {
            if (!input) 
                continue;
            if (find_attachment(prev.getColorOutputs(), input)) {
                return true;
            }
            if (find_attachment(prev.getResolveOutputs(), input)) {
                return true;
            }
        }

        if (same_attachment(next.getDepthStencilInput(), prev.getDepthStencilInput()) || same_attachment(next.getDepthStencilInput(), prev.getDepthStencilOutput())) 
            return true;
        
        for (auto *input : next.getAttachmentInputs()) {
            if (find_attachment(prev.getColorOutputs(), input)) {
                return true;
            }
            if (find_attachment(prev.getResolveOutputs(), input)) {
                return true;
            }
            if (input && prev.getDepthStencilOutput() == input) {
                return true;
            }
        }

        // END SUCCESS CASES

        // the logic concludes by not merging unless we found an explicit reason to merge
        return false;
    };

    // now we merge physical passes using an algorithm outlined by Themaister 
    physicalPasses_.clear();

    for (unsigned int index = 0; index < passStack_.size(); ) {
        auto &pass = passes_[passStack_[index]];

        unsigned int mergeEnd = index + 1; // see if we can merge with the next few passes 
        for (; mergeEnd < passStack_.size(); mergeEnd++) {
            // look for merge candidates until should_merge fails
            bool merge = true;
            for (unsigned int mergeStart = index; mergeStart < mergeEnd; mergeStart++) {
                if (!should_merge(*passes_[passStack_[mergeStart]], *passes_[passStack_[mergeEnd]])) {
                    merge = false;
                    break;
                }
            }

            if (!merge)
                break;
        }

        // after we've found some range of passes to merge, we merge them
        std::vector<unsigned int> passIndices = {};
        passIndices.insert(passIndices.end(), passStack_.begin() + index, passStack_.begin() + mergeEnd);

        // create the physical pass
        auto tmp = shared_from_this();
        physicalPasses_.push_back(std::make_shared<Backend::PhysicalPass>(passIndices, tmp, context_));
        index = mergeEnd;
    }
}

void RenderGraph::buildBarriers() {
    // TODO
}

void RenderGraph::buildAliases() {
    // set up synchronized render target aliasing, where we reuse non-overlapping (wrt execution) resources that have the same format/dimensions (using I think
    // some VkEvent magic to synchronize access)

    // hypothetically this step could be optional but it seems like a good idea for especially larger graphs with lots of resources that
    // might only be used for a few passes out of many; even though i'm aiming for relatively simple graphics, it would be really cool to support
    // even huge fancy AAA pipelines with tons of different resources and passes being used for many things (and not have the framerate tank) 
    
}

#pragma endregion BAKE_RG
