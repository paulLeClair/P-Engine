//
// Created by paull on 2022-06-19.
//

#include "VulkanBackend.hpp"

#include "../../../Application/EngineMode/AnimatedModelDemoMode/AnimatedModelDemoMode.hpp"
#include "../Renderer/VulkanRenderer/VulkanRenderer.hpp"
#include "VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp"
#include "VulkanShaderModule/VulkanShaderModule.hpp"
#include "../Renderer/PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp"

namespace pEngine::girEngine::backend::vulkan {
    VulkanBackend::VulkanBackend(const CreationInput &createInfo) : allocator(VK_NULL_HANDLE),
                                                                    threadPoolHandle(createInfo.threadPoolHandle),
                                                                    imageAcquiredSemaphore(VK_NULL_HANDLE),
                                                                    renderCompleteSemaphore(VK_NULL_HANDLE) {
        applicationContext = std::make_shared<appContext::vulkan::VulkanApplicationContext>(
            appContext::vulkan::VulkanApplicationContext::CreationInput{
                createInfo.vulkanAppName,
                createInfo.vulkanEngineName,
                createInfo.enabledInstanceExtensions,
                createInfo.enabledInstanceLayers,
                createInfo.enabledDeviceExtensions,
                createInfo.enabledDeviceLayers,
                createInfo.physicalDeviceLabel,
                createInfo.logicalDeviceLabel,
                createInfo.vulkanAppVersion,
                createInfo.vulkanEngineVersion,
                createInfo.vulkanApiVersion,
                createInfo.swapchainPresentMode,
                createInfo.numberOfSwapchainImages,
                createInfo.desiredSwapchainImageFormat,
                createInfo.swapchainImageColorSpace,
                createInfo.swapchainImageUsages,
                createInfo.swapchainImagePreTransform,
                createInfo.swapchainShouldClipObscuredTriangles,
                createInfo.initialWindowWidth,
                createInfo.initialWindowHeight
            });

        initializeVmaAllocator();

        // initialize presentation engine
        initializeRendererSemaphores();
        if (!imageAcquiredSemaphore || !renderCompleteSemaphore) {
            // TODO -> log!
        }

        presentationEngine = std::make_unique<VulkanPresentationEngine>(
            VulkanPresentationEngine::CreationInput{
                applicationContext->getLogicalDevice()->getVkDevice(),
                applicationContext->getLogicalDevice()->getGraphicsQueue(), // hardcoding graphics&present in 1 queue
                applicationContext->getOSInterface()->getSwapchain(),
                DEFAULT_SWAPCHAIN_TIMEOUT_IN_MS,
                imageAcquiredSemaphore,
            }
        );

        renderer = std::make_unique<VulkanRenderer>(
            VulkanRenderer::CreationInput{
                applicationContext,
                allocator,
                *presentationEngine,
                createInfo.swapchainPresentMode,
                createInfo.swapchainCompositeAlphaFlag,
                {0.0, 0.0, 0.0, 0.0}, // clear color
            });
    }

    VulkanBackend::~VulkanBackend() {
        // TODO -> properly fill in this backend dtor! there's a ton of crap that needs to be properly obliterated...
        for (auto &shader: vulkanShaderModules) {
            vkDestroyShaderModule(applicationContext->getLogicalDevice()->getVkDevice(), shader.getVkShaderModule(),
                                  nullptr);
        }
    }

    void VulkanBackend::createShaderModules(
        const std::vector<gir::SpirVShaderModuleIR> &shaderModuleGirs) {
        vulkanShaderModules.reserve(shaderModuleGirs.size());
        for (const auto &shaderModuleGir: shaderModuleGirs) {
            vulkanShaderModules.emplace_back(VulkanShaderModule::CreationInput{
                applicationContext->getLogicalDevice()->getVkDevice(),
                shaderModuleGir.getFilename(),
                shaderModuleGir.uid,
                shaderModuleGir.getUsage(),
                shaderModuleGir.getEntryPointName()
            });
        }
    }

    void VulkanBackend::buildRenderGraph(const gir::RenderGraphIR &renderGraph) {
        if (vulkanShaderModules.empty()) {
            // TODO - log!
            return;
        }

        for (auto &renderPass: renderGraph.dynamicRenderPasses) {
            bakeVulkanDynamicRenderPass(const_cast<DynamicRenderPassIR &>(renderPass));
        }
        // temporary measure: hook in the render passes directly to the renderer
        // NOTE: we're assuming at this point that the resource repository is good to go
        renderer->initializeRenderData(vulkanRenderPasses, &resourceRepository);
    }

    void VulkanBackend::bakeVulkanDynamicRenderPass(DynamicRenderPassIR &renderPass) {
        // prepare shader modules
        const std::vector<const VulkanShaderModule *> shaderModules = acquireShaderModulesForRenderPass(renderPass);

        // TODO - collect the push constants from a render pass (not used for animated model demo)

        // ALSO TODO -> move program creation outside of the pass so we can open up the possibility
        // of reusing programs across passes
        const auto newPassProgram = createProgramForRenderPass(renderPass, shaderModules);

        const auto newPassGraphicsPipeline = createGraphicsPipelineForDynamicRenderPass(renderPass, newPassProgram);

        // for now I'll hardcode that all render passes will request the pre-existing swapchain image handles automatically
        std::vector<VkRenderingAttachmentInfo> colorAttachments = {};
        for (uint32_t swapchainImageIndex = 0; swapchainImageIndex < presentationEngine->getSwapchain().
                                               getSwapchainImages().size(); swapchainImageIndex++) {
            auto imageViewPtr = presentationEngine->getSwapchain().getSwapchainImageView(swapchainImageIndex);
            if (!imageViewPtr) {
                // TODO -> log!
                return;
            }

            colorAttachments.push_back(VkRenderingAttachmentInfo{
                VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                nullptr,
                imageViewPtr,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // I think this is ok?
                VK_RESOLVE_MODE_NONE, // TODO -> support multisample
                VK_NULL_HANDLE, // TODO -> support multisample
                VK_IMAGE_LAYOUT_UNDEFINED, // TODO -> support multisample
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_STORE, // TODO -> allow this to be configured
                {0.0, 0.0, 0.0, 0.0} // TODO -> make clear color non-hardcoded
            });
        }

        std::vector<VulkanRenderPass::VertexInputBinding> passInputBindings(renderPass.vertexInputBindings.size());
        unsigned bindingIndex = 0;
        for (auto &binding: renderPass.vertexInputBindings) {
            std::vector<VulkanRenderPass::VertexBindingAttribute> attributes(binding.attributes.size());
            unsigned attributeIndex = 0;
            for (auto &attribute: binding.attributes) {
                attributes[attributeIndex] = {
                    attribute.attributeShaderLocation,
                    (VkFormat) attribute.attributeFormat,
                    attribute.attributeByteOffset
                };
                attributeIndex++;
            }

            passInputBindings[bindingIndex] = {bindingIndex, attributes};
            bindingIndex++;
        }

        std::vector<VulkanModel *> models = {};
        std::vector<VulkanBufferSuballocationHandle> bufferHandles = {};
        for (auto &model: renderPass.models) {
            // since we're pre-baking models in the resource repository we should just grab them here
            auto modelPtr = resourceRepository.obtainModel(model.uid);
            if (!modelPtr) {
                // TODO - log!
                return;
            }
            models.push_back(modelPtr);

            // handle model's uniform buffers (only supported buffer type ATOW)
            for (auto &buffer: model.buffers) {
                bufferHandles.push_back(resourceRepository.obtainBufferHandle(buffer.uid));
            }
        }

        // TODO -> obtain handles to uniform buffers here
        for (auto &bufferAttachment: renderPass.uniformBuffers) {
            bufferHandles.push_back(resourceRepository.obtainBufferHandle(bufferAttachment.attachedBuffer.uid));
        }

        auto depthStencilImageId = renderPass.dynamicRenderPassState.depthStencilAttachmentFormat.first;
        // TODO -> untangle this with proper nullchecking and such
        auto depthAttachmentImageView = resourceRepository.obtainImage(depthStencilImageId)->getImageView();

        if (depthAttachmentImageView == VK_NULL_HANDLE) {
            // TODO -> log!
        }

        auto depthAttachmentRenderingInfo = VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = depthAttachmentImageView,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = {1.0, 1.0, 1.0}
        };


        auto passRenderingInfo = VkRenderingInfo{
            VK_STRUCTURE_TYPE_RENDERING_INFO,
            nullptr,
            0,
            // TODO -> add appropriate configuration for VkRenderingFlags - not sure if it's impt for this demo
            presentationEngine->getDefaultSwapchainImageRenderArea(),
            1, // TODO -> support layering of resources
            0, // TODO -> support multiview
            // NOTE -> all attachments are now being passed in separately (temporarily anyway) because of variable scope concerns
        };

        // NEW: for now, we'll just assume dynamic resources are specified per-pass and will apply to any resources
        // contained within, no matter where they specifically come from (eg a particular model)

        // TODO -> any other resource types that we want to be dynamic
        std::vector<DynamicResourceBinding> dynamicBuffers = {};
        std::vector<DynamicResourceBinding> dynamicImages = {};

        for (auto &dynamicBinding: renderPass.dynamicUniformBuffers) {
            dynamicBuffers.emplace_back(
                dynamicBinding.resourceId,
                dynamicBinding.updateCallback,
                dynamicBinding.priority
            );
        }
        for (auto &dynamicBinding: renderPass.dynamicImages) {
            dynamicImages.emplace_back(
                dynamicBinding.resourceId,
                dynamicBinding.updateCallback,
                dynamicBinding.priority
            );
        }

        // add the new render pass to the backend (they'll be executed in the same order they're added)
        // (until we do the render graph rewrite stuff)
        vulkanRenderPasses.push_back(
            std::make_shared<VulkanDynamicRenderPass>(
                VulkanDynamicRenderPass{
                    renderPass.name + "Vulkan Dynamic Render Pass",
                    renderPass.uid,
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    applicationContext->getInstance(),
                    allocator,
                    newPassGraphicsPipeline,
                    resourceRepository,
                    dynamicBuffers,
                    dynamicImages,
                    passRenderingInfo,
                    // NOTE -> for now, color attachments will just be the swapchain images that the pass must swap between
                    // (in the near future, we'll want to better formalize the render target API and support non-swapchain images)
                    colorAttachments,
                    depthAttachmentRenderingInfo,
                    resourceRepository.obtainImage(depthStencilImageId)
                        ? resourceRepository.obtainImage(depthStencilImageId)->getImage()
                        : VK_NULL_HANDLE,
                    {},
                    models,
                    passInputBindings,
                    newPassProgram,
                    bufferHandles,
                    // TODO -> remove any args that just grab something from resource repo now that it gets passed in
                    resourceRepository.uniformBufferSuballocator.getAllocation(),
                    applicationContext->getGraphicsQueueFamilyIndex(), // TODO -> support compute as well
                    static_cast<uint32_t>(presentationEngine->getSwapchain().getSwapchainImages().size()),
                    applicationContext->getGraphicsQueue() // TODO -> support multiple frames in flight
                }
            )
        );
    }

    std::vector<const VulkanShaderModule *>
    VulkanBackend::acquireShaderModulesForRenderPass(const RenderPassIR &renderPass) const {
        std::vector<const VulkanShaderModule *> shaderModules = {};
        // collect vertex shader
        shaderModules.emplace_back(findAttachedShader(renderPass.vertexShader));

        // TODO - add other shader types as applicable (KISSing for now)

        // collect fragment shader
        shaderModules.emplace_back(findAttachedShader(renderPass.fragmentShader));

        return shaderModules;
    }

    std::shared_ptr<VulkanProgram>
    VulkanBackend::createProgramForRenderPass(const RenderPassIR &renderPass,
                                              const std::vector<const VulkanShaderModule *> &shaderModules) const {
        // for the single-model demo we'll only be supporting vertex and frag shader
        const VulkanShaderModule *vertexShaderModule = {};
        const VulkanShaderModule *fragmentShaderModule = {};
        for (auto &module: shaderModules) {
            if (module->getUsage() == gir::ShaderModuleIR::ShaderUsage::VERTEX_SHADER) {
                vertexShaderModule = module;
                continue;
            }
            if (module->getUsage() == gir::ShaderModuleIR::ShaderUsage::FRAGMENT_SHADER) {
                fragmentShaderModule = module;
                continue;
            }
        }

        return std::make_shared<VulkanProgram>(VulkanProgram::CreationInput{
            renderPass.name + "VulkanProgram",
            UniqueIdentifier(),
            applicationContext->getLogicalDevice()->getVkDevice(),
            threadPoolHandle ? threadPoolHandle->get_thread_count() : 1,
            vertexShaderModule,
            fragmentShaderModule,
            renderPass.drawAttachments, // TODO - refactor the draw attachment/commands stuff in the gir part
            renderPass.vertexInputBindings
        });
    }

    std::shared_ptr<pipeline::VulkanGraphicsPipeline>
    VulkanBackend::createGraphicsPipelineForDynamicRenderPass(const RenderPassIR &renderPass,
                                                              const std::shared_ptr<VulkanProgram> &
                                                              newPassProgram) {
        std::vector<VkVertexInputAttributeDescription> vertexAttribs = newPassProgram->
                getVertexInputAttributeDescriptions();
        // TODO -> reconcile how we handle the attribs here with the new vertex input binding stuff

        auto vertexInputState = VkPipelineVertexInputStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(newPassProgram->getVertexInputBindingDescriptions().size()),
            newPassProgram->getVertexInputBindingDescriptions().data(),
            static_cast<uint32_t>(vertexAttribs.size()),
            vertexAttribs.data()
        };

        // TODO -> wire in color blend attachment config; for now we just hardcode to match dear imgui
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT,

        };

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            // TODO -> un-hardcode the single color attachment
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
        };

        return std::make_shared<pipeline::VulkanGraphicsPipeline>(
            pipeline::VulkanGraphicsPipeline::CreationInput{
                renderPass.name + "GraphicsPipeline",
                UniqueIdentifier(),
                applicationContext->getLogicalDevice()->getVkDevice(),
                newPassProgram->getPipelineLayout(),
                getPipelineCreateFlags(&renderPass),
                getPipelineShaderStages(&renderPass),
                vertexInputState,
                getPipelineInputAssemblyCreateInfo(&renderPass),
                getTessellationStateCreateInfo(&renderPass),
                getViewportStateCreateInfo(&renderPass),
                getRasterizationStateCreateInfo(&renderPass),
                getMultisampleStateCreateInfo(&renderPass),
                getDepthStencilStateCreateInfo(&renderPass),
                colorBlendStateCreateInfo,
                getDynamicStateCreateInfo(&renderPass),
                getPipelineRenderingCreateInfo(&renderPass)
            });
    }

    void VulkanBackend::buildRenderGraphs(
        const std::vector<gir::RenderGraphIR> &renderGraphGirs) {
        for (const auto &renderGraph: renderGraphGirs) {
            buildRenderGraph(renderGraph);
        }
    }

    GraphicsBackend::BakeResult
    VulkanBackend::bakeGirs(const gir::generator::GirGenerator::BakeOutput &girList) {
        const std::vector<gir::DrawAttachmentIR> drawCommandGirs = girList.drawAttachments;
        const std::vector<gir::camera::CameraGIR> cameraGirs = girList.cameras; // TODO - wire these in

        // temporary measure until render graph refactor: prepare swapchain images and hand them to repository
        std::vector<gir::ImageIR> swapchainImageGirs = {};
        std::vector<VkImage> swapchainImageHandles = {};
        uint32_t swapchainImageIndex = 0;
        for (auto &swapchainImage: presentationEngine->getSwapchain().getSwapchainImages()) {
            swapchainImageGirs.emplace_back(gir::ImageIR::CreationInput{
                "Swapchain Image " + std::to_string(swapchainImageIndex),
                UniqueIdentifier(),
                gir::GIRSubtype::IMAGE,
                gir::ImageIR::ImageUsage::SWAPCHAIN_IMAGE,
                static_cast<const gir::resource::FormatIR>(presentationEngine->getSwapchain().swapchainImageFormat),
                nullptr,
                0,
                presentationEngine->getSwapchainImageExtent().width,
                presentationEngine->getSwapchainImageExtent().height,
                false,
                false,
                boost::optional<uint32_t>(swapchainImageIndex)
            });

            swapchainImageHandles.push_back(
                swapchainImage
            );

            swapchainImageIndex++;
        }

        // build up resource repository
        resourceRepository = VulkanResourceRepository(VulkanResourceRepository::CreationInput{
            applicationContext->getLogicalDevice()->getVkDevice(),
            applicationContext->getLogicalDevice()->getGraphicsQueueFamilyIndex(),
            allocator,
            girList.buffers,
            girList.images,
            girList.shaderConstants,
            girList.models,
            applicationContext->getLogicalDevice()->getMinimumUniformBufferAlignment()
        });

        createShaderModules(girList.shaders);

        buildRenderGraphs(girList.renderGraphs);

        for (auto &light: girList.pointLights) {
            // TODO
        }

        return BakeResult::SUCCESS;
    }

    GraphicsBackend::DrawFrameResult VulkanBackend::drawFrame() {
        const auto result = renderer->renderFrame();
        return result == VulkanRenderer::RenderFrameResult::SUCCESS
                   ? DrawFrameResult::SUCCESS
                   : DrawFrameResult::FAILURE;
    }

    std::vector<VkPipelineShaderStageCreateInfo>
    VulkanBackend::getPipelineShaderStages(const RenderPassIR *renderPassIr) const {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};

        // minor todo -> support custom shader stage bit, in case you want to force a particular setting
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->vertexShader,
                                                           VK_SHADER_STAGE_VERTEX_BIT);
        addShaderModulePipelineShaderStageCreateInfoToList(shaderStages,
                                                           renderPassIr->fragmentShader,
                                                           VK_SHADER_STAGE_FRAGMENT_BIT);

        return shaderStages;
    }

    void VulkanBackend::addShaderModulePipelineShaderStageCreateInfoToList(
        std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
        const gir::ShaderModuleIR &shaderAttachment,
        const VkShaderStageFlagBits stageFlags) const {
        const VulkanShaderModule *shaderModule = findAttachedShader(shaderAttachment);
        if (!shaderModule) {
            // TODO - log!
            return;
        }

        shaderStages.push_back(VkPipelineShaderStageCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0, // TODO - evaluate when you even need to set these lol... seems niche but i may be wrong
            stageFlags,
            shaderModule->getVkShaderModule(),
            shaderModule->getShaderEntryPointName().c_str(),
            &shaderModule->getSpecializationInfo()
        });
    }

    VkPipelineCreateFlags
    VulkanBackend::getPipelineCreateFlags(const RenderPassIR *renderPassGir) {
        // TODO - support any useful extensions that make use of these flags
        return 0;
    }

    VkPipelineVertexInputStateCreateInfo
    VulkanBackend::getVertexInputStateCreateInfo(
        const std::shared_ptr<VulkanProgram> &renderPassProgram) {
        // OKAY all this might have to change because we're now tracking attributes again
        std::vector<VkVertexInputAttributeDescription> vertexAttribs = {};

        // however, we do seem to be able to grab things right out of the program, so maybe that'll be the default
        return VkPipelineVertexInputStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            nullptr,
            0,
            static_cast<uint32_t>(renderPassProgram->getVertexInputAttributeDescriptions().size()),
            reinterpret_cast<const VkVertexInputBindingDescription *>(renderPassProgram->
                getVertexInputAttributeDescriptions().data()),
        };
    }

    VkPipelineInputAssemblyStateCreateInfo
    VulkanBackend::getPipelineInputAssemblyCreateInfo(const RenderPassIR *pass) {
        // mostly TODO
        return VkPipelineInputAssemblyStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            nullptr,
            0,
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TODO - obtain this from the render pass
            VK_FALSE // TODO - obtain this from the render pass
        };
    }

    VkPipelineTessellationStateCreateInfo
    VulkanBackend::getTessellationStateCreateInfo(const RenderPassIR *pass) {
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
    VulkanBackend::getViewportStateCreateInfo(const RenderPassIR *pass) const {
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
    VulkanBackend::getRasterizationStateCreateInfo(const RenderPassIR *pass) {
        return VkPipelineRasterizationStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            nullptr,
            0,
            VK_FALSE, // TODO - get from render pass
            VK_FALSE, // TODO - get from render pass
            VK_POLYGON_MODE_FILL, // TODO - get from render pass
            VK_CULL_MODE_NONE, // TODO - get from render pass
            VK_FRONT_FACE_COUNTER_CLOCKWISE, // TODO - acquire this
            pass->depthBiasEnabled ? VK_TRUE : VK_FALSE,
            0.0, // TODO - obtain this
            0.0, // TODO - obtain this
            0.0, // TODO - obtain this
            1.0, // TODO - make this configurable
        };
    }

    VkPipelineMultisampleStateCreateInfo
    VulkanBackend::getMultisampleStateCreateInfo(const RenderPassIR *pass) {
        return VkPipelineMultisampleStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            nullptr,
            0,
            VK_SAMPLE_COUNT_1_BIT, // TODO - obtain from render pass
            VK_FALSE, // TODO - obtain from render pass
            0, // TODO - obtain from render pass
            nullptr, // TODO - obtain from render pass
            VK_FALSE, // TODO - obtain from render pass
            VK_FALSE // TODO - obtain from render pass
        };
    }

    VkPipelineDepthStencilStateCreateInfo
    VulkanBackend::getDepthStencilStateCreateInfo(const RenderPassIR *pass) {
        // omg is it this?


        VkBool32 depthTestEnable = pass->depthTestEnabled ? VK_TRUE : VK_FALSE;
        VkBool32 depthWriteEnable = pass->depthWriteEnabled ? VK_TRUE : VK_FALSE;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //HARDCODING TEMPORARILY
        // VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        return {
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            nullptr,
            0, // TODO - obtain flags from render pass
            depthTestEnable,
            depthWriteEnable,
            depthCompareOp,
            VK_FALSE, // TODO -> actually wire in the depth bounds test stuff
            VK_FALSE, // TODO -> wire in stencil test stuff
        };
    }

    VkPipelineColorBlendStateCreateInfo
    VulkanBackend::getColorBlendStateCreateInfo(const RenderPassIR *pass) {
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
    VulkanBackend::getDearImguiColorBlendStateCreateInfo(
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
    VulkanBackend::getDynamicStateCreateInfo(const RenderPassIR *pass) {
        // TODO
        return VkPipelineDynamicStateCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            nullptr,
            0, // TODO - obtain from render pass
            0, // TODO - obtain from render pass
            nullptr // TODO - obtain from render pass
        };
    }

    VkPipelineRenderingCreateInfo
    VulkanBackend::getPipelineRenderingCreateInfo(const RenderPassIR *pass) const {
        std::vector<VkFormat> passColorAttachmentFormats = {};
        for (auto &attachment: pass->colorAttachments) {
            // TODO -> for now, only swapchain images are allowed to be color attachments; change this

            // NOTE -> this loop depends on the 1-1 mapping between FormatIR and VkFormat values!
            // do not change that enum class without somehow changing how this works!
            passColorAttachmentFormats.push_back(
                static_cast<VkFormat>(
                    attachment.imageFormat
                )
            );
        }

        const auto depthFormat = (VkFormat) pass->depthAttachmentFormat;
        const VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

        return VkPipelineRenderingCreateInfo{
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            nullptr,
            0, // viewmask will remain 0 until we support multiview!
            // For the single-animated-model demo, we'll assume only 1 color attachment and it will correspond to the swapchain
            1,
            &presentationEngine->getSwapchain().swapchainImageFormat,
            depthFormat,
            stencilFormat // TODO - support stencil attachments fully; omitting for single-animated-model-demo
        };
    }

    const VulkanShaderModule *VulkanBackend::findAttachedShader(const gir::ShaderModuleIR &shaderGirAttachment) const {
        // DEBUG: the problem is that this function is returning nullptr
        for (auto &vulkanShaderModule: vulkanShaderModules) {
            if (vulkanShaderModule.getUid() == shaderGirAttachment.uid) {
                return &vulkanShaderModule; // TODO -> rip out shared pointer
            }
        }
        return nullptr;
    }
}
