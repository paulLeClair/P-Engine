//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>
#include <utility>

#include "../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../lib/bs_thread_pool/BS_thread_pool.hpp"

#include "../ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../Backend.hpp"
#include "VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.hpp"
#include "VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "VulkanTexture/VulkanTexture.hpp"
#include "VulkanPushConstant/VulkanPushConstant.hpp"
#include "VulkanRenderable/VulkanRenderable.hpp"
#include "VulkanProgram/VulkanProgram.hpp"
#include "VulkanResourceRepository/VulkanResourceRepository.hpp"
#include "../Renderer/VulkanRenderer/VulkanRenderer.hpp"

using namespace pEngine::girEngine::scene;

namespace pEngine::girEngine::backend::vulkan {
    static const int DEFAULT_NUMBER_OF_BUFFERED_FRAMES = 3;

    static const int DEFAULT_NUMBER_OF_DESCRIPTOR_SET_SLOTS = 4;

    /**
     * Okay, now we're back to this bad boy again. Time to do a whole bunch of gnarly refactoring...
     *
     * First I guess I should go through and take stock of what's here, but I think to start we
     * can probably just do a big read-through but I should also be considering what stuff
     * should be moved out of the backend and into the scene (and gir) and then passed in;
     *
     * for example, there's probably lots of stuff in the ApplicationContext that needs pulling out.
     * Not to mention descriptor set layouts (which are currently handled in the constructor)
     *
     * Probably once the easy pickins are handled, we can dig into the frame execution controller thing
     * as well. I think moving most of the meat of the logic outside of the backend and into the app makes
     * sense, since we can focus on just writing frame drawing code for each presentation mode and what not.
     * Hopefully that design makes it easier to add the newer stuff (namely ray tracing and what not)
     *
     *
     *
     */
    class VulkanBackend final : public GraphicsBackend {
    public:
        /**
         * New idea: instead of just sticking everything for backend config in the front-end, it probably
         * makes more sense to just put all the main configuration for the application context etc here;
         *
         * This way we can just source everything from some input struct and pass it around
         *
         */
        struct CreationInput {
            std::string vulkanAppName;
            std::string vulkanEngineName;

            // TODO - add actual thread pool creation into the overall engine bootup process
            std::shared_ptr<BS::thread_pool> threadPoolHandle;

            std::vector<appContext::vulkan::VulkanInstance::SupportedInstanceExtension> enabledInstanceExtensions;

            std::vector<appContext::vulkan::VulkanInstance::SupportedLayers> enabledInstanceLayers;

            std::vector<appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension> enabledDeviceExtensions;

            std::vector<appContext::vulkan::VulkanLogicalDevice::SupportedDeviceLayer> enabledDeviceLayers;

            // TODO - expand this eventually to support multiple physical/logical devices
            std::string physicalDeviceLabel;
            std::string logicalDeviceLabel;

            // vulkan version infos (should be created with macros)
            uint32_t vulkanAppVersion;
            uint32_t vulkanEngineVersion;
            uint32_t vulkanApiVersion;

            // I think what makes the most sense is to provide swapchain configuration off the bat like this;
            // at least for now. I'm anticipating to come back and buff some of this stuff up later anyway,
            // we just need to support a single mode of presentation right now.
            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            unsigned numberOfSwapchainImages = 3; // triple buffer by default
            VkFormat desiredSwapchainImageFormat;
            VkColorSpaceKHR swapchainImageColorSpace;
            VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag;
            VkImageUsageFlags swapchainImageUsages{};
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform;
            bool swapchainShouldClipObscuredTriangles = true;

            int initialWindowWidth;
            int initialWindowHeight;
        };

        explicit VulkanBackend(const CreationInput &createInfo) : allocator(VK_NULL_HANDLE),
                                                                  threadPoolHandle(createInfo.threadPoolHandle) {
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

            // coming back to this again
            initializeRenderer(createInfo);
        }

        void initializeVmaAllocator() {
            VmaAllocatorCreateInfo allocatorCreateInfo = {
                    0,
                    applicationContext->getPhysicalDevice(),
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    0,
                    nullptr,//nullable callbacks
                    nullptr,//nullable callbacks
                    nullptr,//nullable callbacks
                    nullptr,//nullable callbacks
                    applicationContext->getInstance(),
                    0,
                    nullptr//nullable callbacks
            };

            if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create VmaAllocator!");
            }
        }

        void initializeRenderer(const CreationInput &createInfo) {
            renderer = std::make_shared<backend::render::vulkan::VulkanRenderer>(
                    backend::render::vulkan::VulkanRenderer::CreationInput{
                            applicationContext,
                            createInfo.swapchainPresentMode,
                            createInfo.swapchainCompositeAlphaFlag
                    });
        }

        ~VulkanBackend() override = default;

        // TODO - evaluate the frame drawing mechanism; may need to redesign it but maybe a simple method works...
        // maybe instead we have an entry function that begins the render loop instead of a function that triggers a whole frame
        DrawFrameResult
        drawFrame() override;

        std::shared_ptr<appContext::ApplicationContext> getApplicationContext() override {
            return std::dynamic_pointer_cast<appContext::ApplicationContext>(applicationContext);
        }

        std::shared_ptr<backend::render::Renderer> getFrameContext() override {
            return std::dynamic_pointer_cast<backend::render::Renderer>(renderer);
        }

        void createShaderModules(const std::vector<std::shared_ptr<gir::ShaderModuleIR> > &shaderModuleGirs);

        void buildRenderGraph(const std::shared_ptr<gir::RenderGraphIR> &renderGraph);

        void buildRenderGraphs(const std::vector<std::shared_ptr<gir::RenderGraphIR> > &renderGraphGirs);

        BakeResult
        bakeGirs(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation> > &girList) override;

        [[nodiscard]] const std::shared_ptr<DearImguiVulkanRenderPass> &getDearImguiRenderPass() const;

        std::shared_ptr<render::vulkan::VulkanRenderer> getRenderer() {
            return renderer;
        }

    private:
        VmaAllocator allocator;

        std::shared_ptr<BS::thread_pool> threadPoolHandle;

        std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext; // make this unique maybe?

        // have to return to this stuff now
        std::shared_ptr<render::vulkan::VulkanRenderer> renderer; // make this unique maybe?

        std::shared_ptr<VulkanResourceRepository> resourceRepository;

        std::vector<std::shared_ptr<VulkanShaderModule> > vulkanShaderModules = {};
        std::vector<std::shared_ptr<VulkanProgram> > vulkanPrograms = {};

        std::vector<std::shared_ptr<pipeline::VulkanGraphicsPipeline> > vulkanGraphicsPipelines;
        // TODO - add other pipelines etc

        std::vector<std::shared_ptr<VulkanRenderPass> > vulkanRenderPasses = {};

        // temporary measure: one single dearimgui render pass
        VkRenderPass guiRenderPass = VK_NULL_HANDLE; // another temp measure: externally store the dear imgui VkRenderPass

        // yet another temp measure: store dear imgui color blend state
        // because it's needed for both the pipeline and the dearimgui pass itself
        const VkPipelineColorBlendAttachmentState dearImguiColorBlendAttachmentState = {
                VK_FALSE,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_MIN,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_FACTOR_ONE,
                VK_BLEND_OP_MIN,
                VK_COLOR_COMPONENT_A_BIT
        };

        std::shared_ptr<DearImguiVulkanRenderPass> dearImguiRenderPass = nullptr;

        // TODO - allow user to configure where in the render graph the built-in GUI pass will go (eg before, after, or custom)

        // TODO - add any other Vulkan objects that have to be baked

        /** BAKE HELPER FUNCTIONS */
        std::shared_ptr<vulkan::VulkanShaderModule> findAttachedShader(
                const std::shared_ptr<ShaderGirAttachment> &sharedPtr) {
            for (auto &vulkanShaderModule: vulkanShaderModules) {
                if (vulkanShaderModule->getUid() == sharedPtr->attachedShaderModuleGir->getUid()) {
                    return vulkanShaderModule;
                }
            }
            return nullptr;
        }

        std::vector<VkPipelineShaderStageCreateInfo>
        getPipelineShaderStages(const std::shared_ptr<RenderPassIR> &renderPassIr);

        VkPipelineCreateFlags getPipelineCreateFlags(const std::shared_ptr<RenderPassIR> &renderPassIr);

        VkPipelineVertexInputStateCreateInfo getVertexInputStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr,
                                                                           const std::shared_ptr<VulkanProgram> &renderPassProgram);

        static VkPipelineInputAssemblyStateCreateInfo
        getPipelineInputAssemblyCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineTessellationStateCreateInfo
        getTessellationStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        VkPipelineViewportStateCreateInfo getViewportStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineRasterizationStateCreateInfo
        getRasterizationStateCreateInfo(const std::shared_ptr<RenderPassIR> &passIr);

        VkPipelineMultisampleStateCreateInfo getMultisampleStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineDepthStencilStateCreateInfo
        getDepthStencilStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineColorBlendStateCreateInfo
        getColorBlendStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineColorBlendStateCreateInfo
        getDearImguiColorBlendStateCreateInfo(
                const std::shared_ptr<gir::renderPass::dearImgui::DearImguiRenderPassIR> &ptr,
                const VkPipelineColorBlendAttachmentState &attachmentState);

        VkPipelineDynamicStateCreateInfo getDynamicStateCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        static VkPipelineRenderingCreateInfo getPipelineRenderingCreateInfo(const std::shared_ptr<RenderPassIR> &ptr);

        void
        addShaderModulePipelineShaderStageCreateInfoToList(std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                                           const std::shared_ptr<ShaderGirAttachment> &shaderAttachment,
                                                           VkShaderStageFlagBits vkShaderStageFlagBits);

        void bakeVulkanDynamicRenderPass(const std::shared_ptr<RenderPassIR> &renderPass);

        std::shared_ptr<pipeline::VulkanGraphicsPipeline>
        createGraphicsPipelineForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass,
                                            const std::shared_ptr<vulkan::VulkanProgram> &newPassProgram);

        std::shared_ptr<VulkanProgram> createProgramForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass,
                                                                  const std::vector<std::shared_ptr<VulkanShaderModule>> &shaderModules);

        std::vector<std::shared_ptr<vulkan::VulkanShaderModule>>
        acquireShaderModulesForRenderPass(const std::shared_ptr<RenderPassIR> &renderPass);

        void bakeDearImguiVulkanRenderPass(
                const std::shared_ptr<gir::renderPass::dearImgui::DearImguiRenderPassIR> &renderPass);
    };
} // namespace PGraphics
