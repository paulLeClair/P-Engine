#include "../../../../include/core/p_render/backend/PhysicalPass.hpp"

#include "../../../../include/core/p_render/render_graph/pass/Pass.hpp"

// graph resources
#include "../../../../include/core/p_render/render_graph/resources/ImageResource.hpp"
#include "../../../../include/core/p_render/render_graph/resources/BufferResource.hpp"

// physical resources
#include "../../../../include/core/p_render/backend/resources/Image.hpp"
#include "../../../../include/core/p_render/backend/resources/Buffer.hpp"
#include "../../../../include/core/p_render/backend/resources/SwapchainImage.hpp"

// program
#include "../../../../include/core/p_render/backend/shaders/Program.hpp"

// subpass
#include "../../../../include/core/p_render/render_graph/pass/Subpass.hpp"

using namespace Backend;

// PhysicalPass::PhysicalPass(PhysicalPassCreateInfo &info) {
// PhysicalPass::PhysicalPass(std::vector<unsigned int> passIndices, std::vector<std::shared_ptr<Pass>> &passes) {
PhysicalPass::PhysicalPass(std::vector<unsigned int> passIndices, std::shared_ptr<RenderGraph> graph, std::shared_ptr<Backend::Context> context) {
    graph_ = graph;
    context_ = context;
    const auto &passes = graph->getPasses();

    /* BIG TODO: clean up a lot of this (physical pass creation) logic; i think there's stuff that can be excised/redone in a simpler way, but i'll wait and
    get this working first */

    // this array will be given to the VkRenderPassCreateInfo later 
    std::vector<VkAttachmentDescription> attachments = {};

    // for this maybe i'll try a local struct
        // not sure what i really need here, this entire thing might not be needed, but i'll leave it for now
    struct ShaderResourceDescription {
        // maybe i could just rename this
        // this is redundant i think, should eventually get rid of it probably
        
        enum ResourceUsage {
            // init value
            Unknown,
            /* BUFFERS */
            // uniforms (read-only by design)
            Uniform = 1 << 0,
            UniformTexel = 1 << 1,
            // storage buffers
            StorageBufferReadOnly = 1 << 2,
            StorageBufferReadWrite = 1 << 3,
            // storage texel buffers
            StorageTexelReadOnly = 1 << 4,
            StorageTexelReadWrite = 1 << 5,
            
            /* IMAGES */
            // storage images
            StorageImageReadOnly = 1 << 6,
            StorageImageReadWrite = 1 << 7,
            // sampled images (i think these are just traditional textures)
            SampledImage = 1 << 8 // i think these can only be read 
        };
        using ResourceUsageFlags = uint32_t;
        ResourceUsageFlags usages = 0;
        
        std::string name = "";

        // not sure what else we need to know... maybe just the usage would be fine?

        // one issue is that i need to know which subpasses use the resource, and (for some resources) we need to know
        // how the subpass uses it 
            // i'm thinking the solution for now can be to base our high-level interface around the subpasses, and then
            // we can just rip out this information directly as the user gives it
        // so you'd have to specify each shader resource used by a subpass as an input/output of the appropriate "variety" 
            // (variety being determined by which interface function you call, eg addUniform or addStorageImageInput)
        // then at this point we can just go through each Graph::Subpass' appropriate arrays of shader resources
        // and determine their usages, which i think we'll use later to build the shader resource subpass dependencies
    
        // PROBLEM: i think we do have to be able to support multiple usages here, and in fact
        // the logic of "continue" whenever we see a physical resource we've already encountered is flawed i think
            // if we have a buffer that one subpass uses as a storage output and some later subpass uses as a storage input
            // and output, we would be only adding the usage of being a storage output and would not really address
            // the multiple usages of it
        // i think we might already just add the usages when we see an existing phys resource, but if not
        // we could just have each physical resource maintain all its usages and update em as you go through
        // the graph resources

    };

    std::vector<ShaderResourceDescription> shaderResourceDescriptions = {};
    // could create another/refresh the "seen indices" set but i don't think that's necessary
        // another note: i might need to be able to support multiple usages... but for now i'll assume one

    // // store indices of physical resources we've already seen
    //     // i think we can use this for both framebuffer and shader resources, since we might use the same image as a texture+coloratt+inputatt 
    std::unordered_set<unsigned int> seenResourceIndices = {}; 
    
    // map ATTACHMENT resource names to Attachment info array indices 
    std::unordered_map<std::string, unsigned int> resourceNamesToAttachmentIndices = {};

    // map SHADER resource names to ShaderResource info array indices
    std::unordered_map<std::string, unsigned int> resourceNamesToShaderResourceIndices = {};

    // // store the physical resource index for each FRAMEBUFFER attachment...
    std::vector<unsigned int> attachmentPhysicalResourceIndices = {};
        // these should correspond 1-1 with the attachments array

    // additional attachment information
    std::vector<std::string> attachmentResourceNames = {};
    std::vector<VkImageAspectFlags> attachmentAspectMasks = {};

    // maybe i should store some sort of info for depth/stencil? 
    bool usesDepthStencil = false;
    std::string depthStencilResourceName = "";

    // // store the physical resource index for each SHADER RESOURCE "attachment" ...
    std::vector<unsigned int> shaderResourcePhysicalResourceIndices = {};
        // these should correspond 1-1 with the shader resources array

    // after the attachments array (plus the descriptor set array) has been built up, we can build
    // descriptions for each Graph::Subpass and add them to the overall list of descriptions
    std::vector<VkSubpassDescription> subpassDescriptions = {};
    std::vector<std::unordered_set<unsigned int>> subpassUsedAttachmentIndices = {};

    // JULY5 - i think i might need to maintain some extra stuff here so we know which resources
    // are read/write/readwrite
        // i guess i'll just store a tuple of read/write index sets
    std::vector<std::tuple<std::unordered_set<unsigned int>, std::unordered_set<unsigned int>>> passReadWriteInfos = {};
        // these sets should contain the physical resource indices of the read/write resources for this pass
    std::unordered_map<unsigned int, unsigned int> subpassReadWriteInfoIndices = {};

    std::vector<SubpassAttachments> subpassAttachmentPointers = {};
    // // store physical resource index for each SHADER RESOURCE (accessed via descriptor set (+ maybe i'll lump in push constants))
    // std::vector<unsigned int> shaderResourcePhysicalResourceIndices = {}; // i hate that this has the word "Resource" twice but meh

    // i could eventually use some kind of temporary hash type mechanism similar to Granite
    // and automatically generate the actual VkRenderPass on the fly but for my 
    // purposes (with static render graphs and generally trying to minimize graphics time while getting 
    // decently pretty results (plus the ability to crank it up or down as you want))
    // i think it'll be okay to just bake them at this point and discourage bake()ing often until
    // i add ways to make that more efficient

    /* BUILD FRAMEBUFFER ATTACHMENTS ARRAY */
    #pragma region BAKE_BUILD_FRAMEBUFFER_ATTACHMENTS
    // maybe i'll just declare some lambdas for this...
    const auto &processColorOutputs = [&](Pass &pass) {
        for (auto i = 0u; i < pass.getColorOutputs().size(); i++) {
            auto *colorOutput = pass.getColorOutputs()[i];
            
            if (seenResourceIndices.find(colorOutput->getPhysicalIndex()) != seenResourceIndices.end()) {
                // i thought 
                continue;
            }
            
            // else store the index
            seenResourceIndices.insert(colorOutput->getPhysicalIndex());

            // and the name
            attachmentResourceNames.push_back(colorOutput->getResourceName());

            bool hasInput = (pass.getColorInputs()[i] != nullptr); // just gonna be explicit

            VkAttachmentDescription desc = {};
            desc.flags = 0; // eventually i'll add support for memory aliasing
            
            // layouts + load/store ops
            // for color attachments (like color outputs) we start and finish in color_att_optimal i think...
                // maybe we set this differently depending on whether there's an input?
                    // if there's no input, we assume the resource hasn't been used yet?
            // might have to go through and figure out where to do layout transitions... 
                // still trying to wrap my brain around making this stuff automatic
                // tbh i might need to go through all the physical passes + resources and set the layouts
                    // i think that would mainly apply to the overall renderpass initial/final layouts
            // actually, since we're only reasoning about each VkRenderPass' initial/final layouts, maybe
            // we can deduce the required layout from the render graph info only... 
                // once we know which layouts are required in which VkRenderPasses (i think layout transitions
                // between subpasses of a renderpass are handled with subpass dependencies... not entirely sure how it works though),
                // we can go through and add the barriers we need to make sure each resource is in the right layout during execution
            if (hasInput) {
                desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            else {
                desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }

            // we want to end up in color attachment layout, this being a color output
            desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            // we obviously want to write to color outputs so we want to make sure we store the result
            desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            // check if this output was registered with an input, and make sure we load the existing co
            if (hasInput) {
                // i'm pretty sure if we have a named input resource we want to use LOAD,
                // except maybe swapchain images but idk probably won't matter
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            else {
                // in this case, we don't have a corresponding color input
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    // i think by default i'll try clearing color attachments upon their first use?
            }

            // for color attachments, we don't care about stencil aspect
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

            // get the format that was set when building the graph 
            desc.format = colorOutput->getAttachmentInfo().format;
            
            // set sample count
            switch (colorOutput->getAttachmentInfo().numSamples) {
                case (2):
                    desc.samples = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case (4):
                    desc.samples = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case (8):
                    desc.samples = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case (16):
                    desc.samples = VK_SAMPLE_COUNT_16_BIT;
                    break;
                default:
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
            }

            // push the attachment into the list for the entire render pass
            attachments.push_back(desc);
            resourceNamesToAttachmentIndices[colorOutput->getResourceName()] = attachments.size() - 1;
            attachmentPhysicalResourceIndices.push_back(colorOutput->getPhysicalIndex());
        }
    };

    const auto &processColorInputs = [&](Pass &pass) {
        // i think these correspond to input attachments,
        // and they should i think consist of both "Attachment" and "Color Input" resources
        const auto addInputAttachment = [&](ImageResource *input) {
            VkAttachmentDescription desc = {};
            desc.flags = 0; // again no aliasing yet

            // set the format using what was given
            desc.format = input->getAttachmentInfo().format;

            // for input attachments, we shouldn't have a corresponding output i think... 
                // otherwise it should i think be considered/registered color input
            desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; 
            desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            switch (input->getAttachmentInfo().numSamples) {
                case (2):
                    desc.samples = VK_SAMPLE_COUNT_2_BIT;
                    break;
                case (4):
                    desc.samples = VK_SAMPLE_COUNT_4_BIT;
                    break;
                case (8):
                    desc.samples = VK_SAMPLE_COUNT_8_BIT;
                    break;
                case (16):
                    desc.samples = VK_SAMPLE_COUNT_16_BIT;
                    break;
                default:
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
            }

            attachments.push_back(desc);
            resourceNamesToAttachmentIndices[input->getResourceName()] = attachments.size() - 1;
            // resourceAttachmentIndices[input->getIndex()] = attachments.size() - 1;
            attachmentPhysicalResourceIndices.push_back(input->getPhysicalIndex());
        };

        for (auto *input : pass.getColorInputs()) {
            if (!input) continue;

            if (seenResourceIndices.find(input->getPhysicalIndex()) != seenResourceIndices.end())
                continue;
            
            seenResourceIndices.insert(input->getPhysicalIndex());

            addInputAttachment(input);
        }

        for (auto *input : pass.getAttachmentInputs()) {
            if (seenResourceIndices.find(input->getPhysicalIndex()) != seenResourceIndices.end())
                continue;

            seenResourceIndices.insert(input->getPhysicalIndex());
            
            addInputAttachment(input);
        }
    };

    const auto &processResolveOutputs = [&](Pass &pass) {
        // i'll come back to these too...

    };

    const auto &processDepthStencil = [&](Pass &pass) {
        // TODO: check to make sure that we haven't already set up a depth buffer 

        bool hasDepthInput = pass.getDepthStencilInput() != nullptr;
        bool hasDepthOutput = pass.getDepthStencilOutput() != nullptr;

        if (hasDepthInput || hasDepthOutput) {
            usesDepthStencil = true;
            VkAttachmentDescription desc = {};

            desc.flags = 0;
            desc.samples = VK_SAMPLE_COUNT_1_BIT;

            // actually i think we can only have one depth/stencil output per pass...
            if (hasDepthInput) {
                // check if we have a read-write attachment
                if (hasDepthOutput) {
                    desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }

                // else we have a read-only depth attachment for this pass
                desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED; 

                desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
            // check for a write-only
            else {
                if (hasDepthOutput) {
                    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

                    desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                }
            }

            // TODO: make it so that if format_has_depth_aspect() returns true, we set this up accordingly; 
                //for now i'll just assume that this
                // uses the same load/store op as the depth aspect... 
                    // i'll come back to this later
            desc.stencilLoadOp = desc.loadOp;
            desc.stencilStoreOp = desc.storeOp;

            ImageResource *resource;
            if (hasDepthInput && hasDepthOutput) {
                resource = pass.getDepthStencilInput(); // should verify that these are in fact the same resource
                // cause i think we can only have 1 depth/stencil attachment per pass
            }
            else {
                // else we have one but not the other, so choose the one we have
                resource = (hasDepthInput) ? pass.getDepthStencilInput() : pass.getDepthStencilOutput();   
            }

            attachments.push_back(desc);
            resourceNamesToAttachmentIndices[resource->getResourceName()] = attachments.size() - 1;
            depthStencilResourceName = resource->getResourceName();
            attachmentPhysicalResourceIndices.push_back(resource->getPhysicalIndex());
        }
    };

    // iterate over each given pass and add attachments to the list
    for (auto passIndex : passIndices) {
        // acquire the pass
        auto &pass = *passes[passIndex];

        // now we add attachments from this pass

        // so far i just realized i'm only processing the framebuffer attachments...
        
        processColorInputs(pass); // this i think should be mainly for input attachments...?
        processColorOutputs(pass);
        processResolveOutputs(pass);
        processDepthStencil(pass);
    }

    // setup access masks for attachments
    attachmentAspectMasks.reserve(attachments.size());
    for (int i = 0; i < attachments.size(); i++) {
        attachmentAspectMasks.push_back(0);
    }
    #pragma endregion BAKE_BUILD_FRAMEBUFFER_ATTACHMENTS

    /* BUILD SUBPASS DESCRIPTIONS */
    #pragma region BAKE_BUILD_SUBPASS_DESCRIPTIONS
    
    const auto &setupReadWriteInfo = [&](Pass &pass) {
        // here i think we can just go through the resources that might be written to and 
        // add them to the read/write set
        std::unordered_set<unsigned int> reads = {};
        
        // add all reads to the set

        for (auto *read : pass.getAttachmentInputs()) {
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getColorInputs()) {
            if (!read) continue;

            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getStorageImageInputs()) {
            if (!read) continue;
            
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getTextureInputs()) {
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getUniformBuffers()) {
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getUniformTexelBuffers()) {
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getStorageTexelBufferInputs()) {
            if (!read) continue;
            reads.insert(read->getPhysicalIndex());
        }

        for (auto *read : pass.getStorageBufferInputs()) {
            if (!read) continue;
            reads.insert(read->getPhysicalIndex());
        }

        if (pass.getDepthStencilInput()) {
            reads.insert(pass.getDepthStencilInput()->getPhysicalIndex());
        }

        std::unordered_set<unsigned int> writes = {};

        // add all writes to the set

        for (auto *write : pass.getColorOutputs()) {
            writes.insert(write->getPhysicalIndex());
        }

        for (auto *write : pass.getResolveOutputs()) {
            writes.insert(write->getPhysicalIndex());
        }

        for (auto *write : pass.getStorageImageOutputs()) {
            writes.insert(write->getPhysicalIndex());
        }

        for (auto *write : pass.getStorageBufferOutputs()) {
            writes.insert(write->getPhysicalIndex());
        }

        for (auto *write : pass.getStorageTexelBufferOutputs()) {
            writes.insert(write->getPhysicalIndex());
        }

        if (pass.getDepthStencilOutput()) {
            writes.insert(pass.getDepthStencilOutput()->getPhysicalIndex());
        }

        passReadWriteInfos.push_back(std::tuple<std::unordered_set<unsigned int>, std::unordered_set<unsigned int>>(reads, writes));
    };

    for (auto &passIndex : passIndices) {
        auto &pass = *passes[passIndex];

        // maybe before we do anything else we just build up this Pass' set of read/write infos
        setupReadWriteInfo(pass);

        auto subpassIndex = 0u;

        // std::shared_ptr<std::vector<VkAttachmentReference>> colorAttachments = {};
        // std::shared_ptr<std::vector<VkAttachmentReference>> inputAttachments = {};
        // std::shared_ptr<std::vector<VkAttachmentReference>> resolveAttachments = {}; // ignoring this for now

        for (auto &subpassPtr : pass.getSubpasses()) {
            auto &subpass = *subpassPtr;

            std::shared_ptr<std::vector<VkAttachmentReference>> colorAttachments = std::make_shared<std::vector<VkAttachmentReference>>();
            std::shared_ptr<std::vector<VkAttachmentReference>> inputAttachments = std::make_shared<std::vector<VkAttachmentReference>>();
            std::shared_ptr<std::vector<VkAttachmentReference>> resolveAttachments = std::make_shared<std::vector<VkAttachmentReference>>(); // ignoring this for now

            // store the index of the parent Pass for this subpass
            subpassReadWriteInfoIndices[subpassIndex] = passIndex;

            VkAttachmentReference depthStencilRef = {};

            // we'll have to fill the preserve attachments (which is kinda annoying i think)
                // basically it just means that we have to go through and put
                // any unused attachments as preserve attachments, otherwise
                // vulkan might allow things to happen to them that you wouldn't want
            std::unordered_set<unsigned int> usedAttachmentIndices = {}; // used to build up preserve attachments later
            std::vector<unsigned int> preserveAttachments = {};

            for (auto resourceName : subpass.getColorInputs()) {
                if (resourceName.empty()) continue;
                auto resourceIndex = resourceNamesToAttachmentIndices[resourceName];
                const auto &resource = *pass.getColorInputs()[resourceIndex]; 
                usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resourceName]);

                VkAttachmentReference ref = {};
                ref.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resourceName]);

                // i hope it's possible to just set layouts like this real simply...
                    // may have to handle special cases where things need to have GENERAL layout
                    // etc 
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    // apparently you use color attachment for these input atts too 

                inputAttachments->push_back(ref);
                attachmentAspectMasks[resourceNamesToAttachmentIndices[resourceName]] |= VK_IMAGE_ASPECT_COLOR_BIT;
            }

            for (auto resourceName : subpass.getAttachmentInputs()) {
                auto resourceIndex = resourceNamesToAttachmentIndices[resourceName];
                const auto &resource = *pass.getAttachmentInputs()[resourceIndex];
                usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resourceName]);

                VkAttachmentReference ref = {};

                ref.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resourceName]);
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                inputAttachments->push_back(ref);
                attachmentAspectMasks[resourceNamesToAttachmentIndices[resourceName]] |= VK_IMAGE_ASPECT_COLOR_BIT;
            }

            for (auto resourceName : subpass.getColorOutputs()) {
                auto resourceIndex = resourceNamesToAttachmentIndices[resourceName];
                const auto &resource = *pass.getColorOutputs()[resourceIndex];
                usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resourceName]);

                VkAttachmentReference ref = {};

                ref.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resourceName]);
                ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                colorAttachments->push_back(ref);
                attachmentAspectMasks[resourceNamesToAttachmentIndices[resourceName]] |= VK_IMAGE_ASPECT_COLOR_BIT;
            }

            bool hasDSInput = subpass.getDepthStencilInput() != "";
            bool hasDSOutput = subpass.getDepthStencilOutput() != "";

            if (hasDSInput || hasDSOutput) {
                if (hasDSInput && hasDSOutput) {
                    // read-write
                        // we assume here that the depth/stencil resource has been configured in a sane way
                        // ie i can just arbitrarily take the depth stencil input and it should be correct
                    const auto &resource = *pass.getDepthStencilInput();
                    usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                    attachmentAspectMasks[resourceNamesToAttachmentIndices[resource.getResourceName()]] |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; // TODO: check format to set one and/or the other

                    depthStencilRef.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                    depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else {
                    // read-only
                    if (hasDSInput) {
                        const auto &resource = *pass.getDepthStencilInput();
                        usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                        attachmentAspectMasks[resourceNamesToAttachmentIndices[resource.getResourceName()]] |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; // TODO: check format to set one and/or the other

                        depthStencilRef.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                        
                        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                    }
                    // write-only
                    else {
                        const auto &resource = *pass.getDepthStencilOutput();
                        usedAttachmentIndices.insert(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                        attachmentAspectMasks[resourceNamesToAttachmentIndices[resource.getResourceName()]] |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT; // TODO: check format to set one and/or the other

                        depthStencilRef.attachment = static_cast<uint32_t>(resourceNamesToAttachmentIndices[resource.getResourceName()]);
                        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    }
                }
            }

            // build up list of preserve attachments
            preserveAttachments.clear();
            // actually wait, maybe you can just go through the list of attachments and if the subpass
            // doesn't use the attachment, you add it to the preserve attachments list...
            for (uint32_t i = 0; i < attachments.size(); i++) {
                if (usedAttachmentIndices.find(i) == usedAttachmentIndices.end()) {
                    preserveAttachments.push_back(i);
                }
            }

            // need to keep these alive until the end of physical pass creation i think
            subpassAttachmentPointers.push_back({
                colorAttachments,
                inputAttachments,
                resolveAttachments
            });

            // build up the subpass description
            VkSubpassDescription desc = {};

            // put together the subpass description and add it to the vector
            desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            // set color attachments 
            desc.colorAttachmentCount = static_cast<uint32_t>(colorAttachments->size());
            desc.pColorAttachments = (colorAttachments->size() > 0) ? colorAttachments->data() : nullptr;

            // set resolve (TODO)
            desc.pResolveAttachments = nullptr; // no msaa yet
            
            // set input attachments
            desc.inputAttachmentCount = static_cast<uint32_t>(inputAttachments->size());
            desc.pInputAttachments = (inputAttachments->size() > 0) ? inputAttachments->data() : nullptr;

            // set depth/stencil
            desc.pDepthStencilAttachment = (hasDSInput || hasDSOutput) ? &depthStencilRef : nullptr;

            // set preserve
            desc.preserveAttachmentCount = static_cast<uint32_t>(preserveAttachments.size());
            desc.pPreserveAttachments = (preserveAttachments.size() > 0) ? preserveAttachments.data() : nullptr; 

            subpassDescriptions.push_back(desc);
            subpassUsedAttachmentIndices.push_back(usedAttachmentIndices);
        }

        // increment our subpass index tracker (this is ugly)
        subpassIndex++;
    }
    #pragma endregion BAKE_BUILD_SUBPASS_DESCRIPTIONS

    // ideally at this point, we should have a full list of subpass descriptions
        // this gives us information about the FRAMEBUFFER ATTACHMENTS, but we still know nothing about resources that are 
        // accessed by shaders using descriptors, so i think i'll add a new step to set up a global list of "shader resources" maybe

    /* BUILD SHADER RESOURCE DESCRIPTIONS */
    #pragma region BAKE_BUILD_SHADER_RESOURCE_DESCRIPTIONS

    std::vector<std::string> shaderResourceNames = {};

    for (auto &passIndex : passIndices) {
        auto &pass = passes[passIndex];

        // not really sure if i need to get usage info when we build up the list of shader resources but idk
        
        // i think i need to reset the "seenResourceIndices" thing...
        seenResourceIndices.clear();
            // might not have much of an effect though, since there shouldn't usually be much overlap,
            // except when we have a self-dependency or whatever

        const auto &checkBufferResource = [&](BufferResource *resource, ShaderResourceDescription::ResourceUsageFlags usages) {
            if (seenResourceIndices.find(resource->getPhysicalIndex()) != seenResourceIndices.end()) {
                auto shaderResourceIndex = resourceNamesToShaderResourceIndices[resource->getResourceName()];
                
                shaderResourceDescriptions[shaderResourceIndex].usages |= usages;
                return;
            }
            seenResourceIndices.insert(resource->getPhysicalIndex());
            shaderResourceNames.push_back(resource->getResourceName());

            ShaderResourceDescription desc = {};

            // set up whatever info is required (for now there's just the usage)
            desc.usages |= usages;

            desc.name = resource->getResourceName();

            shaderResourceDescriptions.push_back(desc);
            
            // shaderResourceIndices[resource->getIndex()] = shaderResourceDescriptions.size() - 1;
            resourceNamesToShaderResourceIndices[resource->getResourceName()] = shaderResourceDescriptions.size() - 1;
            shaderResourcePhysicalResourceIndices.push_back(resource->getPhysicalIndex());
        };

        const auto &checkImageResource = [&](ImageResource *resource, ShaderResourceDescription::ResourceUsageFlags usages) {
            if (seenResourceIndices.find(resource->getPhysicalIndex()) != seenResourceIndices.end()) {
                auto shaderResourceIndex = resourceNamesToShaderResourceIndices[resource->getResourceName()];
                
                shaderResourceDescriptions[shaderResourceIndex].usages |= usages;
                return;
            }
            seenResourceIndices.insert(resource->getPhysicalIndex());
            shaderResourceNames.push_back(resource->getResourceName());

            ShaderResourceDescription desc = {};

            // set up whatever info is required (for now there's just the usage)
            desc.usages |= usages;

            desc.name = resource->getResourceName();

            shaderResourceDescriptions.push_back(desc);
            resourceNamesToShaderResourceIndices[resource->getResourceName()] = shaderResourceDescriptions.size() - 1;
            shaderResourcePhysicalResourceIndices.push_back(resource->getPhysicalIndex());
        };

        for (auto &uniform : pass->getUniformBuffers()) {
            checkBufferResource(uniform, ShaderResourceDescription::ResourceUsage::Uniform);
        }

        for (auto &uniformTexel : pass->getUniformTexelBuffers()) {
            // if (seenResourceIndices.find(uniformTexel->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     // i dont know how much danger we 
            //     continue;
            // }
            // seenResourceIndices.insert(uniformTexel->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // // set up whatever info is required
            // desc.usages |= ShaderResourceDescription::ResourceUsage::UniformTexel;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(uniformTexel->getPhysicalIndex());
            checkBufferResource(uniformTexel, ShaderResourceDescription::ResourceUsage::UniformTexel);
        }

        // int index = 0;
        for (auto &storageOutput : pass->getStorageBufferOutputs()) {
            // if (seenResourceIndices.find(storageOutput->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(storageOutput->getPhysicalIndex());
        
            // ShaderResourceDescription desc = {};

            // // first have to check the corresponding input slot in the pass
            //     // actually i don't think we even do...
            // // if (pass->getStorageBufferInputs()[index]) {
            // //     // in this case we have a read-write 
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageBufferReadWrite;
            // // }
            // // else {
            // //     // the resource was registered with no corresponding input, so we have read-only
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageBufferReadOnly;
            // // }

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageBufferReadWrite;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(storageOutput->getPhysicalIndex());
            // index++;

            checkBufferResource(storageOutput, ShaderResourceDescription::ResourceUsage::StorageBufferReadWrite);
        }

        for (auto &storageInput : pass->getStorageBufferInputs()) {
            // i think we check here for any read-only ones, since the read-write ones should have been
            // identified in the above loop where we go over storage buffer outputs
            // if (seenResourceIndices.find(storageInput->getPhysicalIndex()) != seenResourceIndices.end()) { 
            //     continue;
            // }
            // seenResourceIndices.insert(storageInput->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageBufferReadOnly;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(storageInput->getPhysicalIndex());

            checkBufferResource(storageInput, ShaderResourceDescription::ResourceUsage::StorageBufferReadOnly);
        }

        // index = 0;
        for (auto &storageTexelOutput : pass->getStorageTexelBufferOutputs()) {
            // if (seenResourceIndices.find(storageTexelOutput->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(storageTexelOutput->getPhysicalIndex());

            // // i think here we also have to check for read=write or read-only
            // ShaderResourceDescription desc = {};

            // // if (pass->getStorageTexelBufferInputs()[index]) {
            // //     // in this case we have a corresponding input so this is a read-write resource
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageTexelReadWrite;
            // // }
            // // else {
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageTexelReadOnly;
            // // }

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageTexelReadWrite;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(storageTexelOutput->getPhysicalIndex());
            // index++;

            checkBufferResource(storageTexelOutput, ShaderResourceDescription::ResourceUsage::StorageBufferReadWrite);
        }

        for (auto &storageTexelInput : pass->getStorageTexelBufferInputs()) {
            // if (seenResourceIndices.find(storageTexelInput->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(storageTexelInput->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageTexelReadOnly;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(storageTexelInput->getPhysicalIndex());
            checkBufferResource(storageTexelInput, ShaderResourceDescription::ResourceUsage::StorageTexelReadOnly);
        }

        // image shader resources

        for (auto &sampledImage : pass->getTextureInputs()) {
            // if (seenResourceIndices.find(sampledImage->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(sampledImage->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // desc.usages |= ShaderResourceDescription::ResourceUsage::SampledImage;

            // shaderResourceDescriptions.push_back(desc);
            // shaderResourcePhysicalResourceIndices.push_back(sampledImage->getPhysicalIndex());
            checkImageResource(sampledImage, ShaderResourceDescription::ResourceUsage::SampledImage);
        }

        // index = 0;
        for (auto &storageImageOutput : pass->getStorageImageOutputs()) {
            // if (seenResourceIndices.find(storageImageOutput->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(storageImageOutput->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // // if (pass->getStorageImageInputs()[index]) {
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageImageReadWrite;
            // // }
            // // else {
            // //     desc.usage = ShaderResourceDescription::ResourceUsage::StorageImageReadOnly;
            // // }

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageImageReadWrite;
            
            // shaderResourceDescriptions.push_back(desc);
            // index++;

            checkImageResource(storageImageOutput, ShaderResourceDescription::ResourceUsage::StorageImageReadWrite);
        }

        for (auto &storageImageInput : pass->getStorageImageInputs()) {
            // if (seenResourceIndices.find(storageImageInput->getPhysicalIndex()) != seenResourceIndices.end()) {
            //     continue;
            // }
            // seenResourceIndices.insert(storageImageInput->getPhysicalIndex());

            // ShaderResourceDescription desc = {};

            // desc.usages |= ShaderResourceDescription::ResourceUsage::StorageImageReadOnly;

            // shaderResourceDescriptions.push_back(desc);
            checkImageResource(storageImageInput, ShaderResourceDescription::ResourceUsage::StorageImageReadOnly);
        }

    }
    #pragma endregion BAKE_BUILD_SHADER_RESOURCE_DESCRIPTIONS

    // then i think you analyze the usages for each resource and add subpass dependencies
    // when subpasses make use of the same resources
        // can use premade "usage/etc_to_stages" and "usage/etc_to_access" or whatever to set up the 
        // stage/access flags in the dependencies automatically (i think)
    // need to definitely think about this more but i'm pretty sure we can
    // track the the first/last usages of each resource, and then add external dependencies to each
    // subpass (by adding a subpass dependency) that waits for all resources which are used before the subpass
        // if that doesn't work i could potentailly just do a naive thing and have subpasses execute sequentially even tho that's wrong i think

    /* BUILD SUBPASS DEPENDENCIES */
    #pragma region BAKE_BUILD_SUBPASS_DEPENDENCIES
    std::vector<VkSubpassDependency> dependencies = {};
        // basically i need to set up the subpass dependencies within the pass AND
        // also go through and set up any external dependencies (with src/dst being VK_SUBPASS_EXTERNAL)

    #pragma region BAKE_BUILD_SUBPASS_DEPENDENCIES_PREP
    // small step: build up a combined list of all subpasses in order
    std::vector<std::shared_ptr<Subpass>> subpasses = {};
    for (auto &passIndex : passIndices) {
        auto &subpassesToAdd = passes[passIndex]->getSubpasses();
        subpasses.insert(subpasses.end(), subpassesToAdd.begin(), subpassesToAdd.end());
        // need to think about how to represent these... a small struct might be required to package up 
        // all the information needed to convert the Graph::Subpass into something we can execute
    }
    assert(subpasses.size() == subpassDescriptions.size());

    std::vector<std::unordered_set<unsigned int>> subpassUsedShaderResourceIndices = {};
    for (auto subpass = 0u; subpass < subpasses.size(); subpass++) {
        // need to build up a set of used shader resource indices for each subpass
        std::unordered_set<unsigned int> usedShaderResourceIndices = {};

        // i think we have to just go through each of the shader resources and add any used ones to the list
        for (auto &res : subpasses[subpass]->getUniformBuffers()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getUniformTexelBuffers()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageBufferOutputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageBufferInputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageImageOutputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageImageInputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getTextureInputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageTexelBufferOutputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        for (auto &res : subpasses[subpass]->getStorageTexelBufferInputs()) {
            usedShaderResourceIndices.insert(resourceNamesToShaderResourceIndices[res]);
        }

        subpassUsedShaderResourceIndices.push_back(usedShaderResourceIndices);
    }
    #pragma endregion BAKE_BUILD_SUBPASS_DEPENDENCIES_PREP

    /* UTILITY LAMBDAS */
    const auto &checkForResource = [&](std::vector<std::string> names, const std::string &name) {
        for (auto itr = names.begin(); itr != names.end(); itr++) {
            if (*itr == name) {
                return true;
            }
        }
        return false;
    };

    const auto &usesResource = [&](std::shared_ptr<Subpass> subpass, const std::string &name) {
        
        // check all shader resource types to 
        if (checkForResource(subpass->getUniformBuffers(), name)) {
            return true;
        }
        
        if (checkForResource(subpass->getUniformTexelBuffers(), name)) {
            return true;
        }

        if (checkForResource(subpass->getStorageBufferInputs(), name)) {
            return true;
        }

        if (checkForResource(subpass->getStorageBufferOutputs(), name)) {
            return true;
        }

        if (checkForResource(subpass->getStorageImageInputs(), name)) {
            return true;
        }

        if (checkForResource(subpass->getStorageImageOutputs(), name)) {
            return true;
        }

        if (checkForResource(subpass->getTextureInputs(), name)) {
            return true;
        }

        return false;
    };

    const auto &getResourceSubpasses = [&](const std::string &name, std::vector<std::shared_ptr<Subpass>> subpasses) -> std::vector<unsigned int> {
        std::vector<unsigned int> resourceSubpasses = {};
        for (auto subpass = 0u; subpass < subpasses.size(); subpass++) {
            // check if the subpass uses this resource at all 
            if (usesResource(subpasses[subpass], name)) {
                resourceSubpasses.push_back(subpass);
            }
        }
        return resourceSubpasses;
    };

    // might want to break things up into some more lambdas here too
    const auto &processInternalDependencies = [&]() {
        // the idea here is to go through all the subpasses for every resource
        // and build a VkSubpassDependency between subpasses when they both make use of the resource
            // need to think about an algorithm for this... i have to make sure i don't miss any 
            // important cases or add any incorrect dependencies, which could murder my whole thing

        // i think the important thing here is to add dependencies between subpasses that both
        // somehow make use of the same renderpass attachment

        // is it considered ugly to have nested lambdas? `\_(;_;)_/`
        const auto &processFramebufferDependencies = [&]() {
            // for each attachment, we can go through and create dependencies as needed?
            // really hope this is possible...
            for (auto att = 0u; att < attachments.size(); att++) {
                // i guess we could just build a list of subpasses that use this attachment;
                // depending how they use it we insert dependencies 
                    // i think vulkan automatically chains dependencies so i can just worry about "adjacent"
                    // ones... (i think)

                auto attachmentPhysicalResourceIndex = attachmentPhysicalResourceIndices[att];

                std::vector<unsigned int> attSubpasses = {}; 
                // assert(subpassUsedAttachmentIndices.size() == subpasses.size());
                for (auto subpass = 0u; subpass < subpasses.size(); subpass++) {
                    if (subpassUsedAttachmentIndices[subpass].find(att) != subpassUsedAttachmentIndices[subpass].end()) {
                        // if the subpass uses this attachment, add it to the list
                        attSubpasses.push_back(subpass);
                    }
                }
                // we should have every subpass that uses the attachment in order

                // build dependencies for this attachment
                for (auto k = 0u; k < attSubpasses.size() - 1; k++) {
                    // since this is for one resource, i think we can add just a single
                    // dependency (unless they both read the resource?)
                    
                    auto &srcSubpass = subpassDescriptions[attSubpasses[k]];
                    auto &dstSubpass = subpassDescriptions[attSubpasses[k+1]];

                    // we need the image usages of the attachment as well...
                        // maybe i can get this from the physical resource?
                    auto &resource = graph_->getPhysicalResources()[attachmentPhysicalResourceIndex];

                    // here we'll figure out the pipeline stages (and access masks?)
                    VkPipelineStageFlags srcStages = 0;
                    VkPipelineStageFlags dstStages = 0;

                    VkAccessFlags srcAccesses = 0;
                    VkAccessFlags dstAccesses = 0;

                    // here we're just handling FRAMEBUFFER ATTACHMENTS actually... i'll have to do
                    // another pass where we handle descriptor set resources and push constants, which will also need dependencies
                    assert(resource->isImage());
                    auto imageResource = std::dynamic_pointer_cast<Image>(resource);
                    // auto usages = imageResource->getUsages();
                        // i don't really think this is what i need, gonna try another method 

                    enum class SubpassFramebufferAttachmentUsage {
                        ColorAttachment,
                        InputAttachment,
                        ReadDepthStencilAttachment,
                        ReadWriteDepthStencilAttachment,
                        Unknown
                    };
                
                    const auto &determineSubpassFramebufferAttachmentUsage = [&](VkSubpassDescription &desc) {

                        std::unordered_set<SubpassFramebufferAttachmentUsage> usages = {};
                        for (int i = 0; i < desc.colorAttachmentCount; i++) {
                            auto &ref = desc.pColorAttachments[i];
                            if (ref.attachment == att) {
                                usages.insert(SubpassFramebufferAttachmentUsage::ColorAttachment);
                                break;
                            }
                        }   

                        // check if we find it before checking the next class of attachments
                        if (usages.size()) {
                            for (int j = 0; j < desc.inputAttachmentCount; j++) {
                                auto &ref = desc.pInputAttachments[j];
                                if (ref.attachment == att) {
                                    usages.insert(SubpassFramebufferAttachmentUsage::InputAttachment);
                                    break;
                                }
                            }
                        }
                    
                        // handle the DS attachment
                        if (usages.size()) {
                            if (desc.pDepthStencilAttachment) {
                                if (desc.pDepthStencilAttachment->attachment == att) {
                                    // i guess here we can check if it's read or readwrite?
                                    // i think this was already figured out when we set the layout, so do that 
                                    usages.insert((desc.pDepthStencilAttachment->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
                                        ? SubpassFramebufferAttachmentUsage::ReadDepthStencilAttachment : SubpassFramebufferAttachmentUsage::ReadWriteDepthStencilAttachment);
                                }
                            }
                        }

                        if (!usages.size()) {
                            throw std::runtime_error("Unable to determine subpass framebuffer attachment usage!");
                        }

                        return usages;
                    };

                    // find source subpass attachment usages
                    auto &srcDesc = subpassDescriptions[attSubpasses[k]];
                    std::unordered_set<SubpassFramebufferAttachmentUsage> &srcUsages = determineSubpassFramebufferAttachmentUsage(srcDesc);

                    // now find the destination subpass attachment usages
                    auto &dstDesc = subpassDescriptions[attSubpasses[k+1]];
                    std::unordered_set<SubpassFramebufferAttachmentUsage> &dstUsages = determineSubpassFramebufferAttachmentUsage(dstDesc);

                    // now i think according to the usages we set up the src/dst stages and accesses
                        // this is for strictly framebuffer attachments as implied by the scope of this lambda
                    const auto &setupStageAccessMasks = [&](VkPipelineStageFlags &stages, VkAccessFlags &accesses, std::unordered_set<SubpassFramebufferAttachmentUsage> &usages) {
                        // setup color attachment stages/usages
                        if (usages.find(SubpassFramebufferAttachmentUsage::ColorAttachment) != usages.end())  {
                            stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                            accesses |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        }

                        // setup input attachment stages/usages
                        if (usages.find(SubpassFramebufferAttachmentUsage::InputAttachment) != usages.end()) {
                            stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                            accesses |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                        }

                        // setup depth/stencil attachment
                        if (usages.find(SubpassFramebufferAttachmentUsage::ReadDepthStencilAttachment) != usages.end()) {
                            stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                            accesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                        }
                        else if (usages.find(SubpassFramebufferAttachmentUsage::ReadWriteDepthStencilAttachment) != usages.end()) {
                            stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                            accesses |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        }
                    };

                    setupStageAccessMasks(srcStages, srcAccesses, srcUsages);
                    setupStageAccessMasks(dstStages, dstAccesses, dstUsages);

                    // add the dependency 
                    VkSubpassDependency dep = {};
                    dep.dependencyFlags = 0; // i'm not entiiirely sure how these flags work or if they're integral to using them; 
                        // i think they just enable some auxiliary behavior that i don't support here yet

                    dep.srcSubpass = attSubpasses[k];
                    dep.dstSubpass = attSubpasses[k+1];

                    dep.srcStageMask = srcStages;
                    dep.dstStageMask = dstStages;

                    dep.srcAccessMask = srcAccesses;
                    dep.dstAccessMask = dstAccesses;

                    dependencies.push_back(dep);
                }
            }
        };

        // here we'll have to handle dependencies for descriptor sets
        const auto &processDescriptorSetDependencies = [&]() {
            // alright so here we're gonna look at descriptor set buffers/images...

            // i'm hoping i can kinda follow the logic for framebuffer attachments, 
            // except we can't base everything around VkSubpassDescriptions that capture
            // all usage info for framebuffer attachments
                // i still might have to go through and set everything up per-subpass for that since
                // we MAY need to know which SUBPASS
            for (auto res = 0u; res < shaderResourceDescriptions.size(); res++) {
                // find all subpasses that use this resource in order
                // std::vector<unsigned int> resourceSubpasses = {};
                auto name = shaderResourceDescriptions[res].name;

                auto resourceSubpasses = getResourceSubpasses(name, subpasses);
                // now we should know which subpasses use the resource

                // i think we can build dependencies!
                if (resourceSubpasses.size() > 1) {
                    for (auto k = 0; k < resourceSubpasses.size() - 1; k++) {
                        // should be possible to just use one dependency, as long as we don't miss anything
                        VkSubpassDependency dep = {};
                        dep.dependencyFlags = 0; // need to figure out how to handle these flags, hoepfully i can 
                            // ignore for now

                        dep.srcSubpass = resourceSubpasses[k];
                        dep.dstSubpass = resourceSubpasses[k+1];

                        VkPipelineStageFlags srcStages = 0;
                        VkAccessFlags srcAccesses= 0;

                        VkPipelineStageFlags dstStages = 0;
                        VkAccessFlags dstAccesses= 0;  

                        // here we go through and add all stages and accesses
                            // these kinda all act very similarly, so this might be a bit too much 
                        const auto &processSrcSubpass = [&](std::shared_ptr<Subpass> &subpass, VkPipelineStageFlags &stages, VkAccessFlags &accesses) {
                            // here we'll add all the stages/accesses for the subpass from the SOURCE point of view
                            
                            if (checkForResource(subpass->getUniformBuffers(), name)) {
                                // add stages/accesses for uniform 

                                // we'll just assume it's used in every stage i think for now
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getUniformTexelBuffers(), name)) {
                                // add stages/accesses for uniform texel
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageBufferOutputs(), name)) {
                                // add stages/accesses for storage buffer output
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageBufferInputs(), name)) {
                                // add stages/accesses for storage buffer input
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageImageOutputs(), name)) {
                                // add stages/accesses for storage image outputs
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageImageInputs(), name)) {
                                // add stages/accesses for storage image inputs
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageTexelBufferOutputs(), name)) {
                                // add stages/accesses for storage texel buffer outputs
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageTexelBufferInputs(), name)) {
                                // add stages/accesses for storage texel buffer 
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getTextureInputs(), name)) {
                                // add stages/accesses for texture inputs 
                                stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                        };

                        const auto &processDstSubpass = [&](std::shared_ptr<Subpass> &subpass, VkPipelineStageFlags &stages, VkAccessFlags &accesses) {
                            // here we'll add all the stages/accesses for the subpass from the DESTINATION point of view
                            if (checkForResource(subpass->getUniformBuffers(), name)) {
                                // add stages/accesses for uniform 

                                // we'll just assume it's used in every stage i think for now
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getUniformTexelBuffers(), name)) {
                                // add stages/accesses for uniform texel
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageBufferOutputs(), name)) {
                                // add stages/accesses for storage buffer output
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageBufferInputs(), name)) {
                                // add stages/accesses for storage buffer input
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageImageOutputs(), name)) {
                                // add stages/accesses for storage image outputs
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageImageInputs(), name)) {
                                // add stages/accesses for storage image inputs
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getStorageTexelBufferOutputs(), name)) {
                                // add stages/accesses for storage texel buffer outputs
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_WRITE_BIT;
                            }

                            if (checkForResource(subpass->getStorageTexelBufferInputs(), name)) {
                                // add stages/accesses for storage texel buffer 
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                            if (checkForResource(subpass->getTextureInputs(), name)) {
                                // add stages/accesses for texture inputs 
                                stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                accesses |= VK_ACCESS_SHADER_READ_BIT;
                            }

                        };

                        processSrcSubpass(subpasses[dep.srcSubpass], srcStages, srcAccesses);
                        processDstSubpass(subpasses[dep.dstSubpass], dstStages, dstAccesses);

                        dep.srcStageMask = srcStages;
                        dep.dstStageMask = dstStages;

                        dep.srcAccessMask = srcAccesses;
                        dep.dstAccessMask = dstAccesses;  
                
                        dependencies.push_back(dep);
                    }
                }
            }

            // not sure anything else needs done
        };

        // here we check for any self-dependencies, where subpasses use the same resource more than once
        // (in one of only a few valid ways)
        const auto &processSelfDependencies = [&]() {
            // here we go through and look for any self dependencies that need to be set up

            for (auto att = 0u; att < attachments.size(); att++) {
                // i donno exactly which cases i need to handle here;
                    // as far as i know, i just need to check for cases where we use something
                    // as both a TEXTURE and (i think) a color attachment
                // i guess i could also look for storage image/color attachment overlap...

                // may only have to go over the framebuffer attachments to do this...

                auto attName = attachmentResourceNames[att];

                // for each subpass that uses this att, have to check if the same att
                // is found with multiple valid usages
                    // right now there is NO sanity checking so the user has to be very careful 
                    // when setting up their graph
                        // (there's nothing stopping you from trying to use an attachment in an invalid way)
                auto &attSubpasses = getResourceSubpasses(attName, subpasses);
                for (auto &subpass : attSubpasses) {
                    // first maybe we can figure out how the subpass uses the attachment
                    enum SubpassAttachmentUsage {
                        ColorAttachment,
                        ResolveColor,
                        InputAttachment,
                        DepthStencilAttachment,
                        Unknown
                    };
                    auto subpassUsage = SubpassAttachmentUsage::Unknown;

                    for (auto i = 0u; i < subpassDescriptions[subpass].colorAttachmentCount; i++) {
                        // check color attachments
                        if (subpassDescriptions[subpass].pColorAttachments[i].attachment == att) {
                            subpassUsage = SubpassAttachmentUsage::ColorAttachment;
                            break;
                        }
                        // TODO: handle resolve outputs here too
                    }

                    if (subpassUsage == SubpassAttachmentUsage::Unknown) {
                        // check input attachments
                        for (auto i = 0u; i < subpassDescriptions[subpass].inputAttachmentCount; i++) {
                            if (subpassDescriptions[subpass].pInputAttachments[i].attachment == att) {
                                subpassUsage = SubpassAttachmentUsage::InputAttachment;
                                break;
                            }
                        }
                    }

                    if (subpassUsage == SubpassAttachmentUsage::Unknown) {
                        // check depth/stencil
                        if (subpassDescriptions[subpass].pDepthStencilAttachment) {
                            if (subpassDescriptions[subpass].pDepthStencilAttachment->attachment == att) {
                                subpassUsage = SubpassAttachmentUsage::DepthStencilAttachment;
                            }
                        }
                    }

                    // look for any self-dependency conditions...
                        // keeping this naive
                    // TODO: reduce code duplication
                    if (checkForResource(subpasses[subpass]->getTextureInputs(), attName)) {
                        // in this case we have to create the proper dependency
                        VkSubpassDependency dep = {};

                        dep.dependencyFlags = 0; // TODO: handle these flags
                        
                        // set this up as a self-dependency
                        dep.srcSubpass = subpass;
                        dep.dstSubpass = subpass;

                        // textures are used before color inputs/outputs, so use the 
                        // texture usage as the source scope
                        dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                        dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                        // i think we have to set the dst mask according to how the attachment is used
                            // TODO: handle resolve here too
                        if (subpassUsage == SubpassAttachmentUsage::ColorAttachment) {
                            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::InputAttachment) {
                            dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::DepthStencilAttachment) {
                            // set access depending on if we're read-write or read-only
                            if (subpasses[subpass]->getDepthStencilOutput() != "") {
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }
                            else {
                                // depth stencil read only
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }

                            dep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::Unknown) {
                            throw std::runtime_error("Unable to determine subpass attachment usage!");
                        }
    
                        dependencies.push_back(dep);
                    }
                    else if (checkForResource(subpasses[subpass]->getStorageImageOutputs(), attName)) {
                        VkSubpassDependency dep = {};

                        dep.dependencyFlags = 0; // TODO: handle these flags

                        // set this up as a self-dependency
                        dep.srcSubpass = subpass;
                        dep.dstSubpass = subpass;

                        // like textures, we use storage images in the shader pipeline stages, 
                        // so set up the source scope like this:
                        dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                        dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

                        // i think we have to set the dst mask according to how the attachment is used
                        if (subpassUsage == SubpassAttachmentUsage::ColorAttachment) {
                            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::InputAttachment) {
                            dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::DepthStencilAttachment) {
                            if (subpasses[subpass]->getDepthStencilOutput() != "") {
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }
                            else {
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }

                            dep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::Unknown) {
                            throw std::runtime_error("Unable to determine subpass attachment usage!");
                        }

                        dependencies.push_back(dep);
                    }
                    else if (checkForResource(subpasses[subpass]->getStorageImageInputs(), attName)) {
                        VkSubpassDependency dep = {};

                        dep.dependencyFlags = 0; // TODO: handle these flags

                        // set this up as a self-dependency
                        dep.srcSubpass = subpass;
                        dep.dstSubpass = subpass;

                        // i think we have to set the dst mask according to how the attachment is used
                        if (subpassUsage == SubpassAttachmentUsage::ColorAttachment) {
                            dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::InputAttachment) {
                            dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                            dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::DepthStencilAttachment) {
                            // set access depending on if we're read-write or read-only
                            if (subpasses[subpass]->getDepthStencilOutput() != "") {
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }
                            else {
                                // depth stencil read only
                                dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                            }

                            dep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        }
                        else if (subpassUsage == SubpassAttachmentUsage::Unknown) {
                            throw std::runtime_error("Unable to determine subpass attachment usage!");
                        }

                        dependencies.push_back(dep);
                    }
                }
            }

            // i don't really think i need to go through the shader resources, since i don't believe it's possible
            // to use shader resources in more than one way
        };

        // maybe after the lambda declarations we'll call them in turn
        processFramebufferDependencies();
        processDescriptorSetDependencies();

        // maybe we should go through and look for self dependencies?
            // i feel like if a subpass uses the same attachment as an Input and Color attachment, (which should be a common case i'd think...?)
            // that subpass will have to have a self dependency (and other such cases would matter to)
                // there's even a section in the spec about subpass self-dependencies
        processSelfDependencies();

    };
    
    // used to be a bunch of old code here, go dig through the commits if needed (it's bad so just don't)

    const auto &processExternalDependencies = [&]() {
        // TODO: refactor this to reduce all the code duplication etc

        /* PREVIOUS USE */
        if (passes[0]->getIndex() > 0) {
            // only need to consider the case where our first Graph::Pass is not the very first overall

            // go through each subpass i suppose
            for (auto subpassIndex = 0u; subpassIndex < subpasses.size(); subpassIndex++) {
                auto &subpass = subpasses[subpassIndex];

                // look at attachment resources used by the subpass
                for (auto &att : subpassUsedAttachmentIndices[subpassIndex]) {
                    // have to look for any other subpasses in the graph that use this
                    // attachment; 
                    auto attName = attachmentResourceNames[att];

                    // i think i have to go through the remaining passes...
                    for (auto &externalPass : graph_->getPasses()) {
                        // gotta skip any passes that are part of this physical pass
                            // gonna be slow about it
                        bool isExternal = true;
                        for (auto &pass : passes) {
                            if (externalPass == pass) {
                                isExternal = false;
                                break;
                            }
                        }
                        if (!isExternal) continue;

                        // look for any subpasses in this pass that use the attachment
                        for (auto &externalSubpass : externalPass->getSubpasses()) {
                            // 
                            auto externalUsage = externalSubpass->getAttachmentUsage(attName);
                            if (externalUsage != Subpass::SubpassAttachmentUsage::UnusedAttachment) {
                                // in this case we have to set up an external subpass dependency
                                VkSubpassDependency dep = {};
                                auto subpassUsage = subpass->getAttachmentUsage(attName);

                                dep.dependencyFlags = 0;

                                dep.srcSubpass = VK_SUBPASS_EXTERNAL;
                                dep.dstSubpass = subpassIndex;
                                
                                // set up dst scope according to subpass usage
                                if (subpassUsage == Subpass::SubpassAttachmentUsage::ColorAttachment) {
                                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::InputAttachment) {
                                    dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::DepthStencil) {
                                    if (subpass->getDepthStencilOutput() != "") {
                                        dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    else {
                                        dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    dep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::ResolveAttachment) {
                                    // TODO: handle this
                                }

                                // big ugly nested if statement to build the dependency
                                    // TODO: make this a little less ugly
                                if (externalUsage == Subpass::SubpassAttachmentUsage::ColorAttachment) {
                                    dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassAttachmentUsage::InputAttachment) {
                                    dep.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassAttachmentUsage::DepthStencil) {
                                    if (externalSubpass->getDepthStencilOutput() != "") {
                                        dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    else {
                                        dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    dep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                                }

                                dependencies.push_back(dep);
                            }
                        }
                    }
                }
            
                // now we look at all the shader resources used by the subpass 
                for (auto &res : subpassUsedShaderResourceIndices[subpassIndex]) {
                    auto resName = shaderResourceNames[res];

                    for (auto &externalPass : graph_->getPasses()) {
                        // gotta skip any passes that are part of this physical pass
                            // gonna be slow about it
                        bool isExternal = true;
                        for (auto &pass : passes) {
                            if (externalPass == pass) {
                                isExternal = false;
                                break;
                            }
                        }
                        if (!isExternal) continue;

                        for (auto &externalSubpass : externalPass->getSubpasses()) {
                            // check the usage
                            auto externalUsage = externalSubpass->getShaderResourceUsage(resName);
                            if (externalUsage != Subpass::SubpassShaderResourceUsage::UnusedShaderResource) {
                                // here we need a dependency
                                VkSubpassDependency dep = {};
                                auto subpassUsage = subpass->getShaderResourceUsage(resName);

                                dep.dependencyFlags = 0;

                                dep.srcSubpass = VK_SUBPASS_EXTERNAL;
                                dep.dstSubpass = subpassIndex;

                                // setup dest scope
                                    // i honestly don't really think this big if is necessary, i dont know if
                                    // we really have to handle each type of shader resource separately
                                if (subpassUsage == Subpass::SubpassShaderResourceUsage::Uniform) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::UniformTexel) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageBufferOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageBufferInput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageImageOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageImageInput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferInput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::SampledImage) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }

                                // setup source scope
                                if (externalUsage == Subpass::SubpassShaderResourceUsage::Uniform) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::UniformTexel) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageBufferOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageBufferInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageImageOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageImageInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::SampledImage) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }

                                dependencies.push_back(dep);
                            }
                        }
                    }
                }
            }
        }

        /* SUBSEQUENT USE */
        if (passes[0]->getIndex() < graph_->getPasses().size() - 1) {
            // only need to consider the case where our first Graph::Pass is not the very last overall

            // this should be fairly similar to the above 

            // go through all subpasses in this physical pass
            for (auto subpassIndex = 0u; subpassIndex < subpasses.size(); subpassIndex++) {
                auto subpass = subpasses[subpassIndex];

                // look at all attachment resources used by the subpass
                    //TODO: make sure that dependencies between color/input attachments and sampled images are set up
                for (auto &att : subpassUsedAttachmentIndices[subpassIndex]) {
                    auto &attName = shaderResourceNames[att];

                    for (auto &externalPass : graph_->getPasses()) {
                        // skip any passes that are in this physicalpass
                        bool isExternal = true;
                        for (auto &pass : passes) {
                            if (externalPass == pass) {
                                isExternal = false;
                                break;
                            }
                        }
                        if (!isExternal) continue;

                        // now have to look at each subpass of the external pass
                        for (auto &externalSubpass : externalPass->getSubpasses()) {
                            auto externalUsage = externalSubpass->getAttachmentUsage(attName);
                            if (externalUsage != Subpass::SubpassAttachmentUsage::UnusedAttachment) {
                                // set up a dependency 
                                VkSubpassDependency dep = {};
                                auto subpassUsage = subpass->getAttachmentUsage(attName);

                                dep.dependencyFlags = 0;

                                dep.srcSubpass = subpassIndex;
                                dep.dstSubpass = VK_SUBPASS_EXTERNAL;

                                // setup source scope
                                if (subpassUsage == Subpass::SubpassAttachmentUsage::ColorAttachment) {
                                    dep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::InputAttachment) {
                                    dep.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::ResolveAttachment) {
                                    // TODO
                                }
                                else if (subpassUsage == Subpass::SubpassAttachmentUsage::DepthStencil) {
                                    if (subpass->getDepthStencilOutput() != "") {
                                        dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    else {
                                        dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }

                                    dep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                                }

                                // setup destination scope
                                if (externalUsage == Subpass::SubpassAttachmentUsage::ColorAttachment) {
                                    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassAttachmentUsage::InputAttachment) {
                                    dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassAttachmentUsage::ResolveAttachment) {
                                   // todo 
                                }
                                else if (externalUsage == Subpass::SubpassAttachmentUsage::DepthStencil) {
                                    if (externalSubpass->getDepthStencilOutput() != "") {
                                        dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    else {
                                        dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                                    }
                                    
                                    dep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                                }
                            }
                        }
                    }
                }

                // now we look at all the shader resources used by the subpass 
                for (auto res : subpassUsedShaderResourceIndices[subpassIndex]) {
                    auto &resName = shaderResourceNames[res];

                    // now we have to go through every other pass and look for dependencies
                        // this is inherently really really slow, but i feel like 
                        // there is no way youd need more than 300 passes and maybe 500 resources
                            // (how the hell would you even keep track of all that? in those cases
                            // calling bake() over and over again would seem silly anyway so idk)
                    for (auto &externalPass : graph_->getPasses()) {
                        // skip any passes that are in this physicalpass
                        bool isExternal = true;
                        for (auto &pass : passes) {
                            if (externalPass == pass) {
                                isExternal = false;
                                break;
                            }
                        }
                        if (!isExternal) continue;

                        // now have to look at each subpass of the external pass
                        for (auto &externalSubpass : externalPass->getSubpasses()) {

                            auto externalUsage = externalSubpass->getShaderResourceUsage(resName);
                            if (externalUsage != Subpass::SubpassShaderResourceUsage::UnusedShaderResource) {
                                // in this case we have to set up an external dependency
                                VkSubpassDependency dep = {};
                                auto subpassUsage = subpass->getShaderResourceUsage(resName);

                                dep.dependencyFlags = 0;

                                // in this case, we set up an external dependency for a subsequent use...
                                dep.srcSubpass = subpassIndex;
                                dep.dstSubpass = VK_SUBPASS_EXTERNAL;

                                // first set the source scope according to the current subpass' usage
                                if (subpassUsage == Subpass::SubpassShaderResourceUsage::Uniform) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::UniformTexel) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::SampledImage) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageBufferOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageBufferInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageImageOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageImageInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferOutput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                else if (subpassUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferInput) {
                                    dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                                }
                                
                                // set destination scope according to usage
                                if (externalUsage == Subpass::SubpassShaderResourceUsage::Uniform) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; 
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::UniformTexel) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageBufferOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageBufferInput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageImageOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageImageInput) {
                                    // for storage image inputs, should i enable read+write? idk i'll leave it for now
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferOutput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::StorageTexelBufferInput) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }
                                else if (externalUsage == Subpass::SubpassShaderResourceUsage::SampledImage) {
                                    dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                                    dep.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                                }

                                // add dependency 
                                dependencies.push_back(dep);
                            }
                        }
                    }
                }
            }
        }
    };

    // call the lambdas to set up all the subpass dependencies hopefully
    processInternalDependencies();
    processExternalDependencies();
    
    #pragma endregion BAKE_BUILD_SUBPASS_DEPENDENCIES

    #pragma region BAKE_INIT_PHYSICAL_SUBPASS_ARRAY
    // this is also where we'll start building the array of physical subpasses
    // physicalSubpasses_.resize(subpasses.size());
    physicalSubpasses_.clear();
    for (int i = 0; i < subpasses.size(); i++) {
        auto newSubpass = std::make_shared<PhysicalSubpass>();
        physicalSubpasses_.push_back(newSubpass);
        
        // have each subpass store its index
        newSubpass->index = i;

        // have each subpass store the number of color attachments it uses 
            // i think this just directly corresponds to color outputs?
        newSubpass->colorAttachmentCount = subpasses[i]->getColorOutputs().size();

        // build a program for this subpass
        physicalSubpasses_[i]->program = std::make_shared<Backend::Program>(context_, subpasses[i]);
    
        // collect some depth stencil information
        if (!subpasses[i]->getDepthStencilInput().empty() || !subpasses[i]->getDepthStencilOutput().empty()) {
            physicalSubpasses_[i]->depthStencilTestEnabled = true;

            if (!subpasses[i]->getDepthStencilOutput().empty()) {
                physicalSubpasses_[i]->depthStencilWriteEnabled = true;
            }
        }

        // do whatever other initial PhysicalSubpass setup!
    }
    #pragma endregion BAKE_INIT_PHYSICAL_SUBPASS_ARRAY

    // after the subpass descriptions + dependencies are complete, we have everything we need to create the VkRenderPass,
    // so do that i guess!

    /* BUILD RENDER PASS */
    #pragma region BAKE_BUILD_RENDER_PASS
    
    // it seems like it isn't setting up the layouts...
        // i might have to add a step here where we go through and make sure that
        // every layout is properly set up if i can't diagnose the issue earlier 
    // in some cases, such as when a resource is used as a color output exclusively in one pass,
    // but which contains data from another pass' output that we don't want to lose, we might have to do some post-processing here maybe
        // ie the resource is used as a color output previously in the pass stack
    

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.flags = 0;

    renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    renderPassCreateInfo.pSubpasses = subpassDescriptions.size() ? subpassDescriptions.data() : nullptr;

    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.size() ? attachments.data() : nullptr;

    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    // renderPassCreateInfo.dependencyCount = dependencies.size();
    renderPassCreateInfo.pDependencies = dependencies.size() ? dependencies.data() : nullptr;

    // create it!
    VkRenderPass tmp;
    if (vkCreateRenderPass(context_->getLogicalDevice(), &renderPassCreateInfo, nullptr, &tmp) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create VkRenderPass!");
    }
    renderPass_ = tmp;

    #pragma endregion BAKE_BUILD_RENDER_PASS

    /* BUILD IMAGE VIEWS AND FRAMEBUFFERS */
    #pragma region BAKE_BUILD_IMAGE_VIEWS_AND_FRAMEBUFFERS

    // now we'll build the image views for all framebuffer attachments 
    imageViews_.clear(); // for now we just redo these every bake()
    imageViews_.resize(attachments.size());
    for (int i = 0; i < attachments.size(); i++) {
        imageViews_[i] = VK_NULL_HANDLE;

        VkImageViewCreateInfo info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        
        // tbh it might not be necessary to use a separate class for image views, but we'll see 
        info.pNext = nullptr;
        info.flags = 0;
        
        // no swizzling 
        info.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };

        // get the previously-set format from the attachments array
        info.format = attachments[i].format;

        // get the actual VkImage from the physical resource (created before the PhysicalPass ctor is called)
        if (graph_->getPhysicalResources()[attachmentPhysicalResourceIndices[i]]->isSwapchainImage()) {
            // create an image view for the swapchain image
            auto physImage = std::dynamic_pointer_cast<Backend::SwapchainImage>(graph_->getPhysicalResources()[attachmentPhysicalResourceIndices[i]]);
            info.image = physImage->getSwapchainImage(); // provide no arg to get the active swapchain image 
        }
        else {
            auto physImage = std::dynamic_pointer_cast<Backend::Image>(graph_->getPhysicalResources()[attachmentPhysicalResourceIndices[i]]);
            info.image = physImage->getImage();
        }

        

        // set subresource range for the image view
        info.subresourceRange.aspectMask = attachmentAspectMasks[i]; // this gets set up while building subpass descriptions 
        info.subresourceRange.baseArrayLayer = 0; 
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        info.subresourceRange.levelCount = VK_REMAINING_ARRAY_LAYERS;

        info.viewType = VK_IMAGE_VIEW_TYPE_2D; // only one type of image view for now

        if (vkCreateImageView(context_->getLogicalDevice(), &info, nullptr, &imageViews_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create image view!");
        } 
    }

    // using the image views for each attachment, build a framebuffer for this physical pass
    VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferCreateInfo.flags = 0; 
    
    // for now, framebuffers will always be the size of the window?
    framebufferCreateInfo.width = context_->getSwapchainImageSize().width;
    framebufferCreateInfo.height = context_->getSwapchainImageSize().height;
    
    // no layers
    framebufferCreateInfo.layers = 1;

    // set the previously-created renderPass for this framebuffer
    framebufferCreateInfo.renderPass = renderPass_;

    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews_.size());
    framebufferCreateInfo.pAttachments = imageViews_.size() ? imageViews_.data() : nullptr;

    // create the framebuffer!
    if (vkCreateFramebuffer(context_->getLogicalDevice(), &framebufferCreateInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create framebuffer!");
    }
    #pragma endregion BAKE_BUILD_IMAGE_VIEWS_AND_FRAMEBUFFERS

    /* BUILD MEMORY BARRIERS */
    #pragma region BAKE_BUILD_BARRIERS
    // here i think i'll have to make sure that images are all transitioned properly BEFORE their first use
    // and AFTER their last use 
        // we can deduce what layout things need to be in by just traversing the graph and using that info
    


    #pragma endregion BAKE_BUILD_BARRIERS

    /* BUILD GRAPHICS PIPELINES */
    #pragma region BAKE_BUILD_GRAPHICS_PIPELINES
    // hopefully i can multithread this part! (that's one of the things Vulkan advertises)
        // i think i'll make this a later change, and keep things single-threaded until i'm sure it all works
    
    const auto &createGraphicsPipeline = [&](std::shared_ptr<PhysicalSubpass> physicalSubpass) {
        
        VkGraphicsPipelineCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0; // vanilla pipeline functionality for nnow

        /* SETUP PIPELINE LAYOUT */
        info.layout = physicalSubpass->program->getPipelineLayout();

        /* SETUP SHADER STAGES */
        std::vector<VkPipelineShaderStageCreateInfo> stageInfos = {};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        if (physicalSubpass->program->getVertexStageInfo()) {
            // add vertex shader
            stageInfos.push_back(*physicalSubpass->program->getVertexStageInfo());

            const auto &setupInputAssemblyState = [&](VkGraphicsPipelineCreateInfo &info, VkPipelineInputAssemblyStateCreateInfo &inputAssemblyCreateInfo) {
                
                inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssemblyCreateInfo.flags = 0;
                inputAssemblyCreateInfo.pNext = nullptr;

                // i think that this will hook into the Scene that the user is supposed to set up, since that should hopefully
                // package up all the high-level geometry information and it should also be able to provide that info
                // to its associated Graph, and we can probably dynamically fill this struct for 
                inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // just gonna use this for now..
                
                info.pInputAssemblyState = &inputAssemblyCreateInfo;
            };

            const auto &setupVertexInputState = [&](VkGraphicsPipelineCreateInfo &info, VkPipelineVertexInputStateCreateInfo &vertexInputCreateInfo) {
                vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputCreateInfo.pNext = nullptr;
                vertexInputCreateInfo.flags = 0;

                // in the future this will also come from the Scene
                    // it might also be possible to reflect it or something? not sure

                info.pVertexInputState = &vertexInputCreateInfo;
            };

            // we'll set up these here too, since they're relevant to the vertex shader
            setupInputAssemblyState(info, inputAssemblyCreateInfo);
            setupVertexInputState(info, vertexInputCreateInfo);

        }
        
        if (physicalSubpass->program->getTessellationControlStage()) {
            stageInfos.push_back(*physicalSubpass->program->getTessellationControlStage());
        }

        if (physicalSubpass->program->getTessellationEvalStage()) {
            stageInfos.push_back(*physicalSubpass->program->getTessellationEvalStage());
        }

        if (physicalSubpass->program->getGeometryStage()) {
            stageInfos.push_back(*physicalSubpass->program->getGeometryStage());
        }

        if (physicalSubpass->program->getFragmentStage()) {
            // add fragment shader
            stageInfos.push_back(*physicalSubpass->program->getFragmentStage());
        }

        
        // technically we need at least one stage for the vertex shader 
        info.stageCount = static_cast<uint32_t>(stageInfos.size());
        info.pStages = (info.stageCount) ? stageInfos.data() : nullptr;

        /* SETUP TESSELLATION STATE */
            // TODO
        VkPipelineTessellationStateCreateInfo tessellationInfo = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
        tessellationInfo.pNext = nullptr;
        tessellationInfo.flags = 0;
        tessellationInfo.patchControlPoints = 0;
        info.pTessellationState = &tessellationInfo;

        /* SETUP MULTISAMPLE STATE */
            // TODO
        VkPipelineMultisampleStateCreateInfo multisampleInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisampleInfo.flags = 0;
        multisampleInfo.pNext = nullptr;

        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE;

        info.pMultisampleState = &multisampleInfo;

        /* SETUP RASTERIZATION STATE */
        VkPipelineRasterizationStateCreateInfo rastState = {};

        rastState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rastState.pNext = nullptr;
        rastState.flags = 0; // no flags
        rastState.rasterizerDiscardEnable = VK_FALSE; // don't want to discard all fragments
        rastState.polygonMode = VK_POLYGON_MODE_FILL; // we'll just always fill for now
        
        // for now we'll just set this to a hardcoded default too
            // tbh changing this seems like an uncommon thing
        rastState.cullMode = VK_CULL_MODE_BACK_BIT;

        // may have to get winding order from the scene later, since that's where most mesh-specific stuff should live
        rastState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // idk i'll just hardcode this too for now

        // setup depth bias stuff
            // TODO
        rastState.depthBiasEnable = VK_FALSE;

        // set line width to usual value
        rastState.lineWidth = 1.0f;

        info.pRasterizationState = &rastState;

        /* SETUP DEPTH/STENCIL */
        // need to enable this only if we're using a depth/stencil input/output?
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencilInfo.pNext = nullptr;
        depthStencilInfo.flags = 0;

        if (usesDepthStencil) { // this local bool is kinda redundant now but i'll leave it 
            // set up the depthstencilinfo accordingly
            auto dsResource = attachments[resourceNamesToAttachmentIndices[depthStencilResourceName]];

            depthStencilInfo.depthTestEnable = (physicalSubpass->depthStencilTestEnabled) ? VK_TRUE : VK_FALSE;
            depthStencilInfo.depthWriteEnable = (physicalSubpass->depthStencilWriteEnabled) ? VK_TRUE : VK_FALSE;
            
            // configure everything!
                // i'm not sure how much we need to expose at a high level, or if we can
                // just use a particular approach for the scope of this app (which will eventually
                // be a game engine, god willing)
            
            depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS; // only draw if the z-coord of the vertex is closer to us
            depthStencilInfo.minDepthBounds = 0.0f;
            depthStencilInfo.maxDepthBounds = 1.0f;

            // for now, no depth-bound testing implemented
            depthStencilInfo.depthBoundsTestEnable = VK_FALSE;

            // TODO - handle stencil test stuff... 
                // i think i should maybe add more to the interface to handle this better,
                // i might just use some very simple logic for now to set this up
            // it should probably be fairly configurable so i'll have to add in a bunch of stuff to make it configurable
            // in a nice way (i think i'll try and have separate interface functions for setting this kind of thing up?)
                // it may also be possible to do it another way but i'm not entirely sure, making it explicit
                // and setting it up that way might a solution
            depthStencilInfo.stencilTestEnable = VK_FALSE;
        }
        else {
            depthStencilInfo.depthTestEnable = VK_FALSE;
        }

        info.pDepthStencilState = &depthStencilInfo;

        /* SETUP DYNAMIC STATE */
            // TODO, will probably require some more interface additions etc
        VkPipelineDynamicStateCreateInfo dynamicInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicInfo.pNext = nullptr;
        dynamicInfo.flags = 0;

        dynamicInfo.dynamicStateCount = 0; 
        dynamicInfo.pDynamicStates = nullptr;
    
        info.pDynamicState = &dynamicInfo;

        /* SETUP COLOR BLEND STATE */
        VkPipelineColorBlendStateCreateInfo colorBlendInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlendInfo.pNext = nullptr;
        colorBlendInfo.flags = 0;
        
        colorBlendInfo.logicOpEnable = VK_FALSE; // can make this configurable later

        // setup color blend attachment state(s)
            // apparently these have to match the fragment shader outputs...
            // can they be reflected or something?
        // actually i'll have to implement blending in the future, might be good
        // to add some stuff to the interface for that
        colorBlendInfo.attachmentCount = physicalSubpass->colorAttachmentCount;
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates = {};
        for (int i = 0; i < physicalSubpass->colorAttachmentCount; i++) {
            VkPipelineColorBlendAttachmentState colorBlendState = {};
            colorBlendState.blendEnable = VK_FALSE; // disable blending for now
            colorBlendAttachmentStates.push_back(colorBlendState);
        }
        
        colorBlendInfo.pAttachments = (physicalSubpass->colorAttachmentCount) ? colorBlendAttachmentStates.data() : nullptr;

        info.pColorBlendState = &colorBlendInfo;

        /* SETUP VIEWPORT STATE */
        VkPipelineViewportStateCreateInfo viewportInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

        // no scissor rectangles yet
        viewportInfo.scissorCount = 1; // have to set this to 1
        // viewportInfo.pScissors = nullptr;
        VkRect2D hardcodedScissor = {};
        hardcodedScissor.offset = {0, 0}; 
        hardcodedScissor.extent = {1, 1};
        viewportInfo.pScissors = &hardcodedScissor;

        // hardcoding for 1 viewport 
        viewportInfo.viewportCount = 1;
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = context_->getSwapchainImageSize().width;
        viewport.height = context_->getSwapchainImageSize().height;
        // hopefully these values work
        viewport.minDepth = 0.1f;
        viewport.maxDepth = 1.0f;
        viewportInfo.pViewports = &viewport;

        info.pViewportState = &viewportInfo;

        /* REMAINING GRAPHICS PIPELINE INFO SETUP */
        info.renderPass = renderPass_;
        info.subpass = physicalSubpass->index;

        info.basePipelineHandle = VK_NULL_HANDLE;

        // finally, create the graphics pipeline!
            // TODO: setup graphics pipeline caches
        auto result = vkCreateGraphicsPipelines(context_->getLogicalDevice(), nullptr, 1, &info, nullptr, &physicalSubpass->pipeline);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create graphics pipeline!");
        }
    };

    for (auto &physicalSubpass : physicalSubpasses_) {
        createGraphicsPipeline(physicalSubpass);
    }

    #pragma endregion BAKE_BUILD_GRAPHICS_PIPELINES

    /* SETUP DESCRIPTOR SETS? */
    // from here i'm not entirely sure, but it might make sense to set up at least the descriptor pools so that
    // at execute() time the PhysicalPass allows command buffers to bind the required resources

}

void PhysicalPass::execute() {
    // this should be a pretty meaty/important function, 
    // will be called in order to render a frame using a particular graph 

    
}

