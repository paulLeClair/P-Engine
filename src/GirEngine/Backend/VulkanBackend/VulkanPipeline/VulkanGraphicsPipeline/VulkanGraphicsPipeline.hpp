//
// Created by paull on 2022-07-09.
//

#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <boost/optional/optional.hpp>

#include "../VulkanPipeline.hpp"

namespace pEngine::girEngine::backend::vulkan::pipeline {
    // TODO - fix this fucked up namespace lol

    /**
     * This class should basically just wrap the VkPipeline I believe, and I think I'll continue following the pattern that
     * the logic for actually obtaining the backend-facing info from the higher-level Scene info will be external
     * to this class
     */
    class VulkanGraphicsPipeline : public VulkanPipeline {
    public:
        struct CreationInput : public VulkanPipeline::CreationInput {
            VkDevice device = VK_NULL_HANDLE;

            VkPipelineLayout layout = VK_NULL_HANDLE;

            VkPipelineCreateFlags pipelineCreateFlags = 0;
            std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages = {};
            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
            VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo = {};
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
            VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
            VkPipelineRenderingCreateInfo renderingCreateInfo = {};

            boost::optional<VkRenderPass> renderPass = boost::none;
            boost::optional<unsigned> subpassIndex = boost::none;
            boost::optional<int32_t> basePipelineIndex = boost::none;
        };

        explicit VulkanGraphicsPipeline(const CreationInput &info) : VulkanPipeline(info), device(info.device),
                                                                     graphicsPipeline(VK_NULL_HANDLE),
                                                                     pipelineLayout(info.layout) {
            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                nullptr,
                0,
                static_cast<uint32_t>(info.pipelineShaderStages.size()),
                info.pipelineShaderStages.data(),
                &info.vertexInputStateCreateInfo,
                &info.inputAssemblyStateCreateInfo,
                &info.tessellationStateCreateInfo,
                &info.viewportStateCreateInfo,
                &info.rasterizationStateCreateInfo,
                &info.multisampleStateCreateInfo,
                &info.depthStencilStateCreateInfo,
                &info.colorBlendStateCreateInfo,
                &info.dynamicStateCreateInfo,
                info.layout,
                info.renderPass.get_value_or(VK_NULL_HANDLE),
                info.subpassIndex.get_value_or(0),
                VK_NULL_HANDLE, // TODO - support this
                info.basePipelineIndex.get_value_or(0)
            };

            if (!info.renderPass.has_value()) {
                graphicsPipelineCreateInfo.pNext = &info.renderingCreateInfo;
            }

            // this is failing! check validation errors
            // issues:
            // 1. shader modules are being destroyed somewhere, need to figure out where that's happening (FIXED? may still be happening somewhere else)
            const auto result = vkCreateGraphicsPipelines(device,
                                                          VK_NULL_HANDLE,
                                                          1,
                                                          &graphicsPipelineCreateInfo,
                                                          nullptr,
                                                          &graphicsPipeline);
            if (result != VK_SUCCESS) {
                // TODO - log
                throw std::runtime_error("Error in VulkanGraphicsPipeline() -> failed to create graphics pipeline!");
            }
        }

        [[nodiscard]] VkPipelineLayout &getPipelineLayout() {
            return pipelineLayout;
        }

        [[nodiscard]] VkPipeline &getHandle() {
            return graphicsPipeline;
        }

    private:
        VkDevice device;

        VkPipelineLayout pipelineLayout;

        VkPipeline graphicsPipeline;
    };
} // namespace PGraphics
