//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>

#include "../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../lib/bs_thread_pool/BS_thread_pool.hpp"

#include "../ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../Backend.hpp"
#include "VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp"
#include "VulkanResourceRepository/VulkanResourceRepository.hpp"


namespace pEngine::girEngine::backend::vulkan {
    static constexpr int DEFAULT_NUMBER_OF_BUFFERED_FRAMES = 3;

    static constexpr int DEFAULT_NUMBER_OF_DESCRIPTOR_SET_SLOTS = 4;

    // TODO -> move these forward decls into a .fwd.hpp file
    class VulkanRenderer;

    class VulkanPresentationEngine;

    struct VulkanRenderPass;

    class DearImguiVulkanRenderPass;

    class VulkanShaderModule;

    class VulkanProgram;

    namespace pipeline {
        class VulkanGraphicsPipeline;
    }

    class VulkanBackend final : public GraphicsBackend {
    public:
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

            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            unsigned numberOfSwapchainImages = 2;
            VkFormat desiredSwapchainImageFormat;
            VkColorSpaceKHR swapchainImageColorSpace;
            VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag;
            VkImageUsageFlags swapchainImageUsages{};
            VkSurfaceTransformFlagBitsKHR swapchainImagePreTransform;
            bool swapchainShouldClipObscuredTriangles = true;

            int initialWindowWidth;
            int initialWindowHeight;
        };

        explicit VulkanBackend(const CreationInput &createInfo);

        void initializeVmaAllocator() {
            const VmaAllocatorCreateInfo allocatorCreateInfo = {
                0,
                applicationContext->getPhysicalDevice(),
                applicationContext->getLogicalDevice()->getVkDevice(),
                0,
                nullptr, //nullable callbacks
                nullptr, //nullable callbacks
                nullptr, //nullable callbacks
                nullptr, //nullable callbacks
                applicationContext->getInstance(),
                0,
                nullptr //nullable callbacks
            };

            if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create VmaAllocator!");
            }
        }

        ~VulkanBackend() override;

        DrawFrameResult drawFrame() override;

        std::shared_ptr<appContext::ApplicationContext> getApplicationContext() override {
            return std::dynamic_pointer_cast<appContext::ApplicationContext>(applicationContext);
        }

        void createShaderModules(const std::vector<gir::SpirVShaderModuleIR> &shaderModuleGirs);

        void buildRenderGraph(const gir::RenderGraphIR &renderGraph);

        void buildRenderGraphs(const std::vector<gir::RenderGraphIR> &renderGraphGirs);

        BakeResult bakeGirs(const gir::generator::GirGenerator::BakeOutput &girList) override;

        VulkanRenderer &getRenderer() const {
            return *renderer;
        }

        void setLightingUniformBufferUid(const UniqueIdentifier &bufferId) {
            if (bufferId.getValue().is_nil()) {
                // TODO -> log!
                return;
            }
            lightingUniformBufferUid = bufferId;
        }

        VulkanPresentationEngine &getPresentationEngine() const {
            return *presentationEngine;
        }

    private:
        VmaAllocator allocator;

        std::shared_ptr<BS::thread_pool> threadPoolHandle;

        std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext; // make this unique maybe?

        std::unique_ptr<VulkanRenderer> renderer; // make this unique maybe?

        std::unique_ptr<VulkanPresentationEngine> presentationEngine;

        // TEMPORARY HACK for model demo
        UniqueIdentifier cameraUniformBufferUid;
        UniqueIdentifier lightingUniformBufferUid;

        VkSemaphore imageAcquiredSemaphore;
        VkSemaphore renderCompleteSemaphore;

        VulkanResourceRepository resourceRepository = {};

        std::vector<VulkanShaderModule> vulkanShaderModules;
        std::vector<std::unique_ptr<VulkanProgram> > vulkanPrograms;

        std::vector<std::unique_ptr<pipeline::VulkanGraphicsPipeline> > vulkanGraphicsPipelines;

        // for the new geometry bindings stuff, I guess it might be cleaner to just have that live in the render pass
        std::vector<std::shared_ptr<VulkanRenderPass> > vulkanRenderPasses;

        // TODO - add any other Vulkan objects that have to be baked

        /** BAKE HELPER FUNCTIONS */
        [[nodiscard]] const VulkanShaderModule *
        findAttachedShader(const gir::ShaderModuleIR &shaderGirAttachment) const;

        std::vector<VkPipelineShaderStageCreateInfo>
        getPipelineShaderStages(const RenderPassIR *renderPassIr) const;

        static VkPipelineCreateFlags getPipelineCreateFlags(const RenderPassIR *renderPassIr);

        static VkPipelineVertexInputStateCreateInfo getVertexInputStateCreateInfo(
            const std::shared_ptr<VulkanProgram> &renderPassProgram);

        static VkPipelineInputAssemblyStateCreateInfo
        getPipelineInputAssemblyCreateInfo(const RenderPassIR *ptr);

        static VkPipelineTessellationStateCreateInfo
        getTessellationStateCreateInfo(const RenderPassIR *ptr);

        VkPipelineViewportStateCreateInfo getViewportStateCreateInfo(const RenderPassIR *ptr) const;

        static VkPipelineRasterizationStateCreateInfo
        getRasterizationStateCreateInfo(const RenderPassIR *pass);

        static VkPipelineMultisampleStateCreateInfo getMultisampleStateCreateInfo(const RenderPassIR *ptr);

        static VkPipelineDepthStencilStateCreateInfo
        getDepthStencilStateCreateInfo(const RenderPassIR *ptr);

        static VkPipelineColorBlendStateCreateInfo
        getColorBlendStateCreateInfo(const RenderPassIR *ptr);

        static VkPipelineColorBlendStateCreateInfo
        getDearImguiColorBlendStateCreateInfo(
            const VkPipelineColorBlendAttachmentState &attachmentState);

        static VkPipelineDynamicStateCreateInfo getDynamicStateCreateInfo(const RenderPassIR *ptr);

        VkPipelineRenderingCreateInfo getPipelineRenderingCreateInfo(const RenderPassIR *ptr) const;

        void
        addShaderModulePipelineShaderStageCreateInfoToList(std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                                           const gir::ShaderModuleIR &shaderAttachment,
                                                           VkShaderStageFlagBits stageFlags) const;

        void bakeVulkanDynamicRenderPass(DynamicRenderPassIR &renderPass);

        std::shared_ptr<pipeline::VulkanGraphicsPipeline>
        createGraphicsPipelineForDynamicRenderPass(const RenderPassIR &renderPass,
                                                   const std::shared_ptr<VulkanProgram> &newPassProgram);


        std::shared_ptr<VulkanProgram> createProgramForRenderPass(const RenderPassIR &renderPass,
                                                                  const std::vector<const VulkanShaderModule *> &
                                                                  shaderModules)
        const;

        std::vector<const VulkanShaderModule *>
        acquireShaderModulesForRenderPass(const RenderPassIR &renderPass) const;

        void initializeRendererSemaphores() {
            VkSemaphoreCreateInfo semaphoreCreateInfo{
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                nullptr,
                0
            };
            auto result = vkCreateSemaphore(
                applicationContext->getLogicalDevice()->getVkDevice(),
                &semaphoreCreateInfo,
                nullptr,
                &renderCompleteSemaphore
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
            }
            result = vkCreateSemaphore(
                applicationContext->getLogicalDevice()->getVkDevice(),
                &semaphoreCreateInfo,
                nullptr,
                &imageAcquiredSemaphore
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
            }
        }
    };
} // namespace PGraphics
