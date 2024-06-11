//
// Created by paull on 2022-06-25.
//

#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "../../../../../Application/ThreadPool/ThreadPool.hpp"
#include "../../VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.hpp"
#include "../VulkanRenderPass.hpp"

namespace pEngine::girEngine::backend::vulkan {

/**
 * This is an implementation of a Vulkan **dynamic** render pass, meaning it will hopefully make use of the
 * new Vk1.3 dynamic passes; to support the older subpass stuff for tiled GPUs I'll implement a separate class
 * similar to this one
 */
    class VulkanDynamicRenderPass : public VulkanRenderPass {
    public:
        struct CreationInput {
            std::string name;
            VkInstance instance;
            VkDevice device;
//            std::shared_ptr<ThreadPool> threadPool;
            VkPipelineLayout pipelineLayout;
            VkRect2D renderArea;
            uint32_t renderInfoViewMask;
            std::vector<VkRenderingAttachmentInfo> colorRenderingAttachmentInfos;
            VkRenderingAttachmentInfo depthRenderingAttachmentInfo;
            VkRenderingAttachmentInfo stencilRenderingAttachmentInfo;
            std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
            std::vector<VkPushConstantRange> pushConstantRanges;
        };

        explicit VulkanDynamicRenderPass(const CreationInput &createInfo);

        VulkanDynamicRenderPass(const VulkanDynamicRenderPass &other)
                : name(other.name),
                  device(other.device),
                  instance(other.instance),
                  graphicsPipeline(other.graphicsPipeline),
//                  threadPool(other.threadPool),
                  renderingInfo(other.renderingInfo) {
        }

        ~VulkanDynamicRenderPass() = default;


        [[nodiscard]] bool isDynamicRenderPass() const override {
            return true;
        }

    private:
        std::string name;

        VkDevice device;
        VkInstance instance;

        std::shared_ptr<pipeline::VulkanGraphicsPipeline> graphicsPipeline;

        VkRenderingInfo renderingInfo;

        std::vector<std::function<void(VkCommandBuffer)>> renderPassDrawCallbacks = {};

        static VkRenderingFlags getRenderingFlags(const VulkanDynamicRenderPass::CreationInput &createInfo);

        uint32_t getPipelineViewMask(const CreationInput &info);

        std::vector<VkFormat> getColorAttachmentFormats(const CreationInput &info);

        VkFormat getDepthAttachmentFormat(const CreationInput &info);

        VkFormat getStencilAttachmentFormat(const CreationInput &info);

        std::vector<VkPipelineShaderStageCreateInfo> getPipelineShaderStageCreateInfos(const CreationInput &info);

        VkPipelineVertexInputStateCreateInfo getPipelineVertexInputStateCreateInfo(const CreationInput &info);

        VkPipelineInputAssemblyStateCreateInfo getPipelineInputAssemblyStateCreateInfo(const CreationInput &info);

        VkPipelineTessellationStateCreateInfo getPipelineTessellationStateCreateInfo(const CreationInput &info);

        VkPipelineViewportStateCreateInfo getPipelineViewportStateCreateInfo(const CreationInput &info);

        VkPipelineRasterizationStateCreateInfo getPipelineRasterizationStateCreateInfo(const CreationInput &info);

        VkPipelineMultisampleStateCreateInfo getPipelineMultisampleStateCreateInfo(const CreationInput &info);

        VkPipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfo(const CreationInput &info);

        VkPipelineColorBlendStateCreateInfo getPipelineColorBlendStateCreateInfo(const CreationInput &info);

        static VkPipelineDynamicStateCreateInfo getPipelineDynamicStateCreateInfo(const CreationInput &info);

        VkPipelineLayout getPipelineLayout(const CreationInput &info);

        void initializeGraphicsPipeline(const CreationInput &createInfo);

        static std::string getGraphicsPipelineNameString(const CreationInput &createInfo);
    };

}// namespace PGraphics
