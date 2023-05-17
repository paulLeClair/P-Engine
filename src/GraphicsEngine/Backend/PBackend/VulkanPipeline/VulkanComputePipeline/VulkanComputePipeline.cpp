//
// Created by paull on 2022-07-09.
//

#include "VulkanComputePipeline.hpp"

namespace PGraphics {

VulkanComputePipeline::VulkanComputePipeline( const VulkanComputePipeline::CreateInfo &info ) : device( info.device ) {
  // mostly a stub for now

  VkComputePipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0 };

  // TODO - create pipeline (stub for now)
}
VulkanComputePipeline::~VulkanComputePipeline() {
  vkDestroyPipeline( device, pipeline, nullptr );
}


}// namespace PGraphics