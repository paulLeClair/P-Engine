//
// Created by paull on 2022-06-19.
//

#include "VulkanBackend.hpp"

namespace pEngine::girEngine::backend {
    void vulkan::VulkanBackend::createShaderModules(
            const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &shaderModuleGirs) {
        for (const auto &shaderModuleGir: shaderModuleGirs) {
            vulkanShaderModules.push_back(std::make_shared<VulkanShaderModule>(VulkanShaderModule::CreationInput{
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    shaderModuleGir->getFilename(),
                    shaderModuleGir->getUid(),
                    shaderModuleGir->getUsage(),
                    shaderModuleGir->getEntryPointName()
            }));
        }
    }

    void vulkan::VulkanBackend::buildRenderGraph(const std::shared_ptr<gir::RenderGraphIR> &renderGraph) {
        // I guess the logic will be to just basically add everything as if we'd be amalgamating all render graphs into one
        // I can change that in the future pretty easily tho

        // I'm thinking it probably makes sense to build the programs first...?
        if (vulkanShaderModules.empty()) {
            // TODO - log!
            return;
        }

        // TODO - important: we'll need to have already set up the swapchain stuff, since render passes will likely
        // need that information to be available (particularly the DearImGui pass)
        // granted, that code is ancient as fuck and maybe we'll end up wanting to encapsulate that stuff differently
        // and refactor it

        for (const auto &renderPass: renderGraph->getRenderPasses()) {
            // TODO - add support for the traditional VkRenderPass & VkSubpass shit, plus any other render pass types as applicable
            if (renderPass->getRenderPassSubtype() == RenderPassIR::RenderPassSubtype::DYNAMIC) {
                bakeVulkanDynamicRenderPass(renderPass);
            }
            if (renderPass->getRenderPassSubtype() == RenderPassIR::RenderPassSubtype::DEAR_IMGUI) {
                // here we bake the dearimgui thing!
                // the backend only stores 1 of these, so we'll have to LOG something if we overwrite it (ie the user included >1 dearimgui pass)
                // in the future we can change it so you could have multiple dearimgui passes that are maybe combined into 1
                bakeDearImguiVulkanRenderPass(
                        std::dynamic_pointer_cast<gir::renderPass::dearImgui::DearImguiRenderPassIR>(renderPass));
            }
        }

    }

    void vulkan::VulkanBackend::bakeVulkanDynamicRenderPass(
            const std::shared_ptr<RenderPassIR> &renderPass) {
        // prepare shader modules
        std::vector<std::shared_ptr<VulkanShaderModule>> shaderModules = acquireShaderModulesForRenderPass(renderPass);
        // janky lil check
        for (const auto &shaderModule: shaderModules) {
            if (!shaderModule) {
                // TODO - log, don't throw
                throw std::runtime_error(
                        "Error in VulkanBackend::buildRenderGraph() -> Null shader module has been constructed!");
            }
        }

        // TODO - collect the push constants from a render pass (I don't think these are incorporated yet into the new backend/gir)

        auto newPassProgram = createProgramForRenderPass(renderPass, shaderModules);

        auto newPassGraphicsPipeline = createGraphicsPipelineForRenderPass(renderPass, newPassProgram);

        // NOTE -> these dynamic passes are currently half-done, they don't need to be fleshed out until we
        // have actual geometry to render ourselves
        auto newRenderPass = std::make_shared<VulkanDynamicRenderPass>(VulkanDynamicRenderPass::CreationInput{
                "label", // TODO
                applicationContext->getInstance(),
                applicationContext->getLogicalDevice()->getVkDevice(),
                newPassProgram->getPipelineLayout(),
                VkRect2D(), // TODO - obtain from window system
                0, // TODO - obtain render info view mask from... somewhere?
                {}, // TODO - obtain color rendering attachment infos
                {}, // TODO - obtain depth rendering attachment infos
                {}, // TODO - obtain stencil rendering attachment infos
                {}, // TODO - obtain descriptor set layouts from program
                {} // TODO - obtain push constant ranges from attached push constants
        });

        // add the new render pass to the backend (they'll be executed in the same order they're added)
        vulkanRenderPasses.push_back(newRenderPass);
    }

    /**
     *
     */
    void vulkan::VulkanBackend::bakeDearImguiVulkanRenderPass(
            const std::shared_ptr<gir::renderPass::dearImgui::DearImguiRenderPassIR> &renderPass) {
        // this should be somewhat similar to baking the dynamic vulkan render pass; however we'll
        // have to prepare a few other things and may need to hook in some sources for those pieces of data
        // prepare shader modules
        std::vector<std::shared_ptr<VulkanShaderModule>> shaderModules = acquireShaderModulesForRenderPass(renderPass);
        // janky lil check
        for (const auto &shaderModule: shaderModules) {
            if (!shaderModule) {
                // TODO - log, don't throw
                throw std::runtime_error(
                        "Error in VulkanBackend::buildRenderGraph() -> Null shader module has been constructed!");
            }
        }

        auto newPassProgram = createProgramForRenderPass(renderPass, shaderModules);

        // TODO - move all this imgui stuff into its own class;
        // this just initializes the dear imgui render pass
        if (!DearImguiVulkanRenderPass::initializeVulkanRenderPass(
                applicationContext->getLogicalDevice()->getVkDevice(),
                applicationContext->getOSInterface()->getSwapchain()->swapchainImageFormat,
                guiRenderPass
        )) {
            // TODO - log!
        }

        auto newPassGraphicsPipeline = std::make_shared<pipeline::VulkanGraphicsPipeline>(
                pipeline::VulkanGraphicsPipeline::CreationInput{
                        renderPass->getName() + "GraphicsPipeline",
                        UniqueIdentifier(),
                        applicationContext->getLogicalDevice()->getVkDevice(),
                        newPassProgram->getPipelineLayout(),
                        getPipelineCreateFlags(renderPass),
                        getPipelineShaderStages(renderPass),
                        getVertexInputStateCreateInfo(renderPass, newPassProgram),
                        getPipelineInputAssemblyCreateInfo(renderPass),
                        getTessellationStateCreateInfo(renderPass),
                        getViewportStateCreateInfo(renderPass),
                        getRasterizationStateCreateInfo(renderPass),
                        getMultisampleStateCreateInfo(renderPass),
                        getDepthStencilStateCreateInfo(renderPass),
                        getDearImguiColorBlendStateCreateInfo(renderPass, dearImguiColorBlendAttachmentState),
                        getDynamicStateCreateInfo(renderPass),
                        {VK_STRUCTURE_TYPE_RENDERING_INFO}, // no rendering info required
                        boost::optional<VkRenderPass>(guiRenderPass),
                        boost::none,
                        boost::none
                });


        std::vector<VkImageView> swapchainImageViews = {};
        for (const VkImage &swapchainImage: applicationContext->getOSInterface()->getSwapchain()->getSwapchainImages()) {
            // construct a swapchain image view for this image
            VkImageView imageView = VK_NULL_HANDLE;
            VkImageViewCreateInfo imageViewCreateInfo{
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    nullptr,
                    0,
                    swapchainImage,
                    VK_IMAGE_VIEW_TYPE_2D, // hardcoded for now
                    applicationContext->getOSInterface()->getSwapchain()->swapchainImageFormat,
                    { // no swizzling (hardcoded for now)
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY
                    },
                    {
                            VK_IMAGE_ASPECT_COLOR_BIT, // color is the important thing I'm guessing?
                            0,// hardcoded no mips for now
                            1,
                            0, // hardcoded no arrays for now
                            1
                    }
            };

            if (vkCreateImageView(
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    &imageViewCreateInfo,
                    nullptr,
                    &imageView) != VK_SUCCESS) {
                // TODO - log!
            }

            swapchainImageViews.push_back(imageView);
        }


        auto newPass = std::make_shared<DearImguiVulkanRenderPass>(DearImguiVulkanRenderPass::CreationInput{
                applicationContext->getInstance(),
                applicationContext->getPhysicalDevice(),
                applicationContext->getLogicalDevice()->getVkDevice(),
                guiRenderPass,
                applicationContext->getOSInterface()->getSwapchain()->getNumberOfSwapchainImages(),
                applicationContext->getOSInterface()->getSwapchain()->swapchainImageFormat,
                applicationContext->getOSInterface()->getSwapchain()->getSwapchainImageExtent(),
                applicationContext->getLogicalDevice()->getGraphicsQueue(), // hardcoded graphics+present queue for now
                applicationContext->getLogicalDevice()->getGraphicsQueueFamilyIndex(),
                swapchainImageViews,
                renderPass->getDearImguiGuiCallbacks(),
#ifdef _WIN32
                applicationContext->getOSInterface()->getWin32Window()->getWinApiHWND()
#endif
        });

        dearImguiRenderPass = newPass;
        // temporary measure: set the baked render pass pointer
        renderer->setDearImguiVulkanRenderPassPointer(dearImguiRenderPass);
    }

    std::vector<std::shared_ptr<vulkan::VulkanShaderModule>>
    vulkan::VulkanBackend::acquireShaderModulesForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass) {
        std::vector<std::shared_ptr<vulkan::VulkanShaderModule>> shaderModules = {};
        // collect vertex shader
        shaderModules.emplace_back(findAttachedShader(renderPass->getVertexShaderAttachment()));

        // collect geometry shader if applicable
        if (renderPass->getGeometryShaderAttachment()) {
            shaderModules.emplace_back(findAttachedShader(renderPass->getGeometryShaderAttachment()));
        }

        // collect tessellation shaders if applicable
        if (renderPass->getTessellationControlShaderAttachment()) {
            if (!renderPass->getTessellationEvaluationShaderAttachment()) {
                // TODO - log! and probably throw!
            }
            shaderModules.emplace_back(findAttachedShader(renderPass->getTessellationControlShaderAttachment()));
        }
        if (renderPass->getTessellationEvaluationShaderAttachment()) {
            if (!renderPass->getTessellationControlShaderAttachment()) {
                // TODO - log! and probably throw!
            }
            shaderModules.emplace_back(findAttachedShader(renderPass->getTessellationEvaluationShaderAttachment()));
        }

        // TODO - add other shader types as applicable (KISSing for now)

        // collect fragment shader
        shaderModules.emplace_back(findAttachedShader(renderPass->getFragmentShaderAttachment()));

        return shaderModules;
    }

    std::shared_ptr<vulkan::VulkanProgram>
    vulkan::VulkanBackend::createProgramForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass,
                                                      const std::vector<std::shared_ptr<VulkanShaderModule>> &shaderModules) {
        return std::make_shared<VulkanProgram>(VulkanProgram::CreationInput{
                renderPass->getName() + "VulkanProgram",
                UniqueIdentifier(),
                threadPoolHandle ? threadPoolHandle->get_thread_count() : 1,
                shaderModules,
                renderPass->getDrawAttachments(), // TODO - refactor the draw attachment/commands stuff in the gir part
                applicationContext->getLogicalDevice()->getVkDevice()
        });
    }

    std::shared_ptr<vulkan::pipeline::VulkanGraphicsPipeline>
    vulkan::VulkanBackend::createGraphicsPipelineForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass,
                                                               const std::shared_ptr<vulkan::VulkanProgram> &newPassProgram) {
        return std::make_shared<pipeline::VulkanGraphicsPipeline>(
                pipeline::VulkanGraphicsPipeline::CreationInput{
                        renderPass->getName() + "GraphicsPipeline",
                        UniqueIdentifier(),
                        applicationContext->getLogicalDevice()->getVkDevice(),
                        newPassProgram->getPipelineLayout(),
                        getPipelineCreateFlags(renderPass),
                        getPipelineShaderStages(renderPass),
                        getVertexInputStateCreateInfo(renderPass, newPassProgram),
                        getPipelineInputAssemblyCreateInfo(renderPass),
                        getTessellationStateCreateInfo(renderPass),
                        getViewportStateCreateInfo(renderPass),
                        getRasterizationStateCreateInfo(renderPass),
                        getMultisampleStateCreateInfo(renderPass),
                        getDepthStencilStateCreateInfo(renderPass),
                        getColorBlendStateCreateInfo(renderPass),
                        getDynamicStateCreateInfo(renderPass),
                        getPipelineRenderingCreateInfo(renderPass)
                });
    }

    void vulkan::VulkanBackend::buildRenderGraphs(
            const std::vector<std::shared_ptr<gir::RenderGraphIR> > &renderGraphGirs) {
        // OKAY so I think the basic render graph bake is in albeit with lots of placeholder and todo
        // now I think I can start worrying about the window system; some details I haven't really thought about
        // (like specifically where the window system plugs into everything else)

        // technically I'm just trying to support one render graph but I'll just stick it in a loop anyway
        for (const auto &renderGraph: renderGraphGirs) {
            buildRenderGraph(renderGraph);
        }
    }

    GraphicsBackend::BakeResult vulkan::VulkanBackend::bakeGirs(
            const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &girList) {
        // alright! let's get this shite goin again.

        // i guess first things first we can bake all the resources and give them to the repository?

        // first separate girs out by their renderPassSubtype
        std::vector<std::shared_ptr<gir::BufferIR> > bufferGirs = {};
        std::vector<std::shared_ptr<gir::ImageIR> > imageGirs = {};
        std::vector<std::shared_ptr<gir::ShaderConstantIR> > shaderConstantGirs = {};
        std::vector<std::shared_ptr<gir::DrawAttachmentIR> > drawCommandGirs = {};
        std::vector<std::shared_ptr<gir::RenderGraphIR> > renderGraphGirs = {};
        std::vector<std::shared_ptr<gir::ShaderModuleIR> > shaderModuleGirs = {};
        for (auto &gir: girList) {
            switch (gir->getSubtype()) {
                case (gir::GIRSubtype::BUFFER): {
                    bufferGirs.push_back(std::dynamic_pointer_cast<gir::BufferIR>(gir));
                    break;
                }
                case (gir::GIRSubtype::IMAGE): {
                    imageGirs.push_back(std::dynamic_pointer_cast<gir::ImageIR>(gir));
                    break;
                }
                case (gir::GIRSubtype::SHADER_CONSTANT): {
                    shaderConstantGirs.push_back(std::dynamic_pointer_cast<gir::ShaderConstantIR>(gir));
                    break;
                }
                case (gir::GIRSubtype::DRAW_COMMANDS): {
                    drawCommandGirs.push_back(std::dynamic_pointer_cast<gir::DrawAttachmentIR>(gir));
                    break;
                }
                case (gir::GIRSubtype::RENDER_GRAPH): {
                    renderGraphGirs.push_back(std::dynamic_pointer_cast<gir::RenderGraphIR>(gir));
                    break;
                }
                case (gir::GIRSubtype::SHADER_MODULE): {
                    shaderModuleGirs.push_back(std::dynamic_pointer_cast<gir::ShaderModuleIR>(gir));
                    break;
                }
                default: {
                    // TODO - log!
                    break;
                }
            }
        }
        // build up resource repository
        resourceRepository = std::make_shared<VulkanResourceRepository>(VulkanResourceRepository::CreationInput{
                *applicationContext->getLogicalDevice(),
                allocator,
                bufferGirs,
                imageGirs,
                shaderConstantGirs
        });

        // then after that we could build the programs (which should handle shaders and end up with a pipeline layout)

        // after that we can use the pipeline layouts to build the graphics pipelines first, and then render passes
        // I think for the initial implementation we can consider keeping it simple but comparing graphics pipeline
        // configurations isn't that hard; idea being we would hand the VulkanRenderPass a VulkanGraphicsPipeline
        // that was created based on its config, and we'd reuse them between compatible render passes

        // one thing: do we need to build the shader modules ahead of time?
        createShaderModules(shaderModuleGirs);

        // on second thought, maybe we can make it so that we just process the render graph all at once
        // and just build the programs with the render passes (and probably also the pipelines)
        buildRenderGraphs(renderGraphGirs);

        // once render passes and graphics pipelines are all figured out we can probably bake the renderables,
        // which iirc are all turned into "draw commands"?
        // TODO - renderables/custom draw commands in a later update! I think we gotta trim fat and aim for imgui only

        // after that... idk maybe we build the frame execution context / window system shite?

        return BakeResult::SUCCESS;
    }

    GraphicsBackend::DrawFrameResult vulkan::VulkanBackend::drawFrame() {
        auto result = renderer->renderFrame();
        return (result == backend::render::vulkan::VulkanRenderer::RenderFrameResult::SUCCESS)
               ? GraphicsBackend::DrawFrameResult::SUCCESS : GraphicsBackend::DrawFrameResult::FAILURE;
    }

    std::vector<VkPipelineShaderStageCreateInfo>
    vulkan::VulkanBackend::getPipelineShaderStages(const std::shared_ptr<RenderPassIR> &renderPassIr) {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};

        // at this point, shader modules should already have been built
        // minor todo -> support custom shader stage bit, in case you want to force a particular setting
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->getVertexShaderAttachment(),
                                                           VK_SHADER_STAGE_VERTEX_BIT);
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->getGeometryShaderAttachment(),
                                                           VK_SHADER_STAGE_GEOMETRY_BIT);
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->getTessellationEvaluationShaderAttachment(),
                                                           VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->getTessellationControlShaderAttachment(),
                                                           VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->getFragmentShaderAttachment(),
                                                           VK_SHADER_STAGE_FRAGMENT_BIT);

        return shaderStages;
    }

    void vulkan::VulkanBackend::addShaderModulePipelineShaderStageCreateInfoToList(
            std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
            const std::shared_ptr<ShaderGirAttachment> &shaderAttachment,
            const VkShaderStageFlagBits vkShaderStageFlagBit) {
        if (shaderAttachment && shaderAttachment->attachedShaderModuleGir) {
            std::shared_ptr<VulkanShaderModule> shaderModule = findAttachedShader(shaderAttachment);
            // TODO - nullptr check for shader module

            shaderStages.push_back(VkPipelineShaderStageCreateInfo{
                    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    nullptr,
                    0, // TODO - evaluate when you even need to set these lol... seems niche but i may be wrong
                    vkShaderStageFlagBit,
                    shaderModule->getVkShaderModule(),
                    shaderModule->getShaderEntryPointName().c_str(),
                    &shaderModule->getSpecializationInfo()
            });
        }
    }

    VkPipelineCreateFlags
    vulkan::VulkanBackend::getPipelineCreateFlags(const std::shared_ptr<RenderPassIR> &renderPassGir) {
        // TODO - support any useful extensions that make use of these flags
        return 0;
    }

    VkPipelineVertexInputStateCreateInfo
    vulkan::VulkanBackend::getVertexInputStateCreateInfo(const std::shared_ptr<RenderPassIR> &renderPassGir,
                                                         const std::shared_ptr<VulkanProgram> &renderPassProgram) {
        return VkPipelineVertexInputStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                nullptr,
                0,
                // OKAY - this one's kinda juicy; where/how do we get the vertex binding descriptions?
                // the input attribute descriptions should be reflected already; we might just need to get these directly from the program/shader modules
                static_cast<uint32_t>(renderPassProgram->getVertexInputAttributeDescriptions().size()),
                reinterpret_cast<const VkVertexInputBindingDescription *>(renderPassProgram->getVertexInputAttributeDescriptions().data()),
                // for the bindings, we have to get that from the high-level scene specification
        };
    }

    /**
     * This one is going to have to be returned to later; when we have multiple vertex input types we'll have to support
     * different primitive topologies (which I think aren't even handled yet) and also
     * @param ptr
     * @return
     */
    VkPipelineInputAssemblyStateCreateInfo
    vulkan::VulkanBackend::getPipelineInputAssemblyCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        return VkPipelineInputAssemblyStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TODO - obtain this from the render pass
                VK_FALSE // TODO - obtain this from the render pass
        };
    }

    VkPipelineTessellationStateCreateInfo
    vulkan::VulkanBackend::getTessellationStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        // TODO - flesh this out when we get to the point of adding tessellation
        return VkPipelineTessellationStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                nullptr,
                0,
                0 // TODO - obtain this from the render pass
        };
    }

    /**
     * This one will also need fleshing out later... particularly when we get to the camera stuff.
     * For the first pass it'll just be viewport-less basically
     * @param ptr
     * @return
     */
    VkPipelineViewportStateCreateInfo
    vulkan::VulkanBackend::getViewportStateCreateInfo(const std::shared_ptr<RenderPassIR> &pass) {
        return VkPipelineViewportStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                nullptr,
                0,
                1,
                &applicationContext->getOSInterface()->getViewport(),
                1,
                &applicationContext->getOSInterface()->getScissor()
        };
    }

    VkPipelineRasterizationStateCreateInfo
    vulkan::VulkanBackend::getRasterizationStateCreateInfo(const std::shared_ptr<RenderPassIR> &passIr) {

        return VkPipelineRasterizationStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_FALSE, // TODO - get from render pass
                VK_FALSE, // TODO - get from render pass
                VK_POLYGON_MODE_FILL, // TODO - get from render pass
                VK_CULL_MODE_BACK_BIT, // TODO - get from render pass
                VK_FRONT_FACE_CLOCKWISE, // TODO - acquire this
                passIr->isDepthBiasEnabled() ? VK_TRUE : VK_FALSE,
                0.0, // TODO - obtain this
                0.0, // TODO - obtain this
                0.0, // TODO - obtain this
                1.0, // TODO - make this configurable
        };
    }

    VkPipelineMultisampleStateCreateInfo
    vulkan::VulkanBackend::getMultisampleStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        return VkPipelineMultisampleStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                nullptr,
                0,
                VK_SAMPLE_COUNT_1_BIT, // TODO - obtain from render pass
                VK_FALSE, // TODO - obtain from render pass
                0,// TODO - obtain from render pass
                nullptr,// TODO - obtain from render pass
                VK_FALSE,// TODO - obtain from render pass
                VK_FALSE// TODO - obtain from render pass
        };
    }

    VkPipelineDepthStencilStateCreateInfo
    vulkan::VulkanBackend::getDepthStencilStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                nullptr,
                0, // TODO - obtain flags from render pass
                VK_FALSE, // TODO - obtain from render pass
                VK_FALSE, // TODO - obtain from render pass
                VK_COMPARE_OP_ALWAYS, // TODO - obtain from render pass
                VK_FALSE,// TODO - obtain from render pass
                VK_FALSE,// TODO - obtain from render pass
                VK_STENCIL_OP_KEEP, // TODO - obtain from render pass
                VK_STENCIL_OP_ZERO,
        };
    }

    VkPipelineColorBlendStateCreateInfo
    vulkan::VulkanBackend::getColorBlendStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        return VkPipelineColorBlendStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                nullptr,
                0, // TODO - obtain from render pass
                VK_FALSE, // TODO - obtain from render pass
                VK_LOGIC_OP_AND, // TODO - obtain from render pass
                0, // TODO - obtain from render pass
                nullptr, // TODO - obtain from render pass
                {} // TODO - obtain from render pass
        };
    }

    VkPipelineColorBlendStateCreateInfo
    vulkan::VulkanBackend::getDearImguiColorBlendStateCreateInfo(
            const std::shared_ptr<gir::renderPass::dearImgui::DearImguiRenderPassIR> &ptr,
            const VkPipelineColorBlendAttachmentState &dearImguiColorBlendAttachmentState) {
        return VkPipelineColorBlendStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                nullptr,
                0, // TODO - obtain from render pass
                VK_FALSE, // TODO - obtain from render pass
                VK_LOGIC_OP_AND, // TODO - obtain from render pass
                1, // TODO - obtain from render pass
                &dearImguiColorBlendAttachmentState, // TODO - obtain from somewhere else
                {} // TODO - obtain from render pass
        };
    }


    VkPipelineDynamicStateCreateInfo
    vulkan::VulkanBackend::getDynamicStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr) {
        return VkPipelineDynamicStateCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                nullptr,
                0, // TODO - obtain from render pass
                0, // TODO - obtain from render pass
                nullptr // TODO - obtain from render pass
        };
    }

    VkPipelineRenderingCreateInfo
    vulkan::VulkanBackend::getPipelineRenderingCreateInfo(const std::shared_ptr<RenderPassIR> &renderPass) {
        return VkPipelineRenderingCreateInfo{
                VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                nullptr,
                0, // TODO - obtain from render pass
                0, // TODO - obtain from render pass
                nullptr, // TODO - obtain from render pass
                VK_FORMAT_UNDEFINED, // TODO - obtain from render pass
                VK_FORMAT_UNDEFINED // TODO - obtain from render pass
        };
    }

    const std::shared_ptr<vulkan::DearImguiVulkanRenderPass> &vulkan::VulkanBackend::getDearImguiRenderPass() const {
        return dearImguiRenderPass;
    }


}
