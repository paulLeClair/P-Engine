//
// Created by paull on 2022-07-09.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include "../VulkanPipeline.hpp"

namespace PGraphics {

/**
 * Similarly to the VulkanGraphicsPipeline class, this is TODO until I make + start an issue to refactor
 * VulkanDynamicRenderPass to not create its own graphics pipeline
 *
 * I might even make some sort of VulkanPipelineManager or something, since they're important and we can make it easier
 * to add stuff like caching/etc
 */
class VulkanComputePipeline : public VulkanPipeline {
public:
  struct CreateInfo {
    VkDevice device;
  };

  explicit VulkanComputePipeline( const CreateInfo &info );
  ~VulkanComputePipeline();

private:
  VkPipeline pipeline;
  VkDevice device;
};

}// namespace PGraphics
