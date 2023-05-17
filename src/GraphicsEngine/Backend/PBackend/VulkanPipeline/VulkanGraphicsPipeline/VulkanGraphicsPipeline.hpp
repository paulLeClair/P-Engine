//
// Created by paull on 2022-07-09.
//

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "../VulkanPipeline.hpp"

namespace PGraphics {

/**
 * This class should basically just wrap the VkPipeline I believe, and I think I'll continue following the pattern that
 * the logic for actually obtaining the backend-facing info from the higher-level Scene info will be external
 * to this class
 */
class VulkanGraphicsPipeline : public VulkanPipeline {
public:
  struct CreateInfo {
    VkDevice device;

    VkPipelineLayout layout;

    VkPipelineCreateFlags pipelineCreateFlags;
    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo;
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    VkPipelineRenderingCreateInfo renderingCreateInfo;
  };

  explicit VulkanGraphicsPipeline( const CreateInfo &info );

private:
  VkDevice device;

  VkPipelineLayout pipelineLayout;

  VkPipeline graphicsPipeline;
};

}// namespace PGraphics
