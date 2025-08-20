//
// Created by paull on 2022-07-09.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include "../VulkanPipeline.hpp"

namespace pEngine::girEngine::backend::vulkan::pipeline {

/**
 * Similarly to the VulkanGraphicsPipeline class, this is TODO until I make + start an issue to refactor
 * VulkanDynamicRenderPass to not create its own graphics pipeline
 *
 * I might even make some sort of VulkanPipelineManager or something, since they're important and we can make it easier
 * to add stuff like caching/etc
 */
    class VulkanComputePipeline : public VulkanPipeline {
    public:
        struct CreationInput : public VulkanPipeline::CreationInput {
            VkDevice device = VK_NULL_HANDLE;
        };

        explicit VulkanComputePipeline(const CreationInput &info) : VulkanPipeline(info) {

        }

    private:
        VkPipeline pipeline;
        VkDevice device;
    };

}// namespace PGraphics
