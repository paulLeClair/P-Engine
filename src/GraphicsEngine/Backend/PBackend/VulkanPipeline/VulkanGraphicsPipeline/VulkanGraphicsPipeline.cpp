//
// Created by paull on 2022-07-09.
//

#include "VulkanGraphicsPipeline.hpp"

#include <stdexcept>

namespace PGraphics {
VulkanGraphicsPipeline::VulkanGraphicsPipeline( const VulkanGraphicsPipeline::CreateInfo& info )
    : device( info.device ),
      pipelineLayout( info.layout ) {
  VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                                      &info.renderingCreateInfo,
                                                      info.pipelineCreateFlags,
                                                      static_cast<uint32_t>( info.pipelineShaderStages.size() ),
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
                                                      pipelineLayout,
                                                      VK_NULL_HANDLE,
                                                      0,
                                                      graphicsPipeline,
                                                      0 };

  VkResult createPipelineResult =
      vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &graphicsPipeline );
  if ( createPipelineResult != VK_SUCCESS ) {
    throw std::runtime_error( "Unable to create graphics pipeline!" );
  }
}
}// namespace PGraphics