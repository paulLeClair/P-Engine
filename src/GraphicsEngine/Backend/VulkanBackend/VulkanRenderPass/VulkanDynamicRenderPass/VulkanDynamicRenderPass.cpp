//
// Created by paull on 2022-06-25.
//

#include "VulkanDynamicRenderPass.hpp"

#include <stdexcept>
#include <vector>

namespace pEngine::girEngine::backend::vulkan {

    VulkanDynamicRenderPass::VulkanDynamicRenderPass(const VulkanDynamicRenderPass::CreationInput &createInfo)
            : device(createInfo.device),
              instance(createInfo.instance),
              name(createInfo.name),
              renderingInfo({}) {

        renderingInfo = {VK_STRUCTURE_TYPE_RENDERING_INFO,
                         nullptr,
                         getRenderingFlags(createInfo),
                         createInfo.renderArea,
                         1,
                         createInfo.renderInfoViewMask,
                         static_cast<uint32_t>( createInfo.colorRenderingAttachmentInfos.size()),
                         createInfo.colorRenderingAttachmentInfos.data(),
                         &createInfo.depthRenderingAttachmentInfo,
                         &createInfo.stencilRenderingAttachmentInfo};

//        initializeGraphicsPipeline(createInfo);
    }

    // TODO - evaluate whether this functionality is needed
    void VulkanDynamicRenderPass::initializeGraphicsPipeline(const VulkanDynamicRenderPass::CreationInput &createInfo) {
        auto colorAttachmentFormats = getColorAttachmentFormats(createInfo);
        auto depthAttachmentFormat = getDepthAttachmentFormat(createInfo);
        auto stencilAttachmentFormat = getStencilAttachmentFormat(createInfo);

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                                                                     nullptr,
                                                                     getPipelineViewMask(createInfo),
                                                                     static_cast<uint32_t>( colorAttachmentFormats.size()),
                                                                     colorAttachmentFormats.data(),
                                                                     depthAttachmentFormat,
                                                                     stencilAttachmentFormat};
        pipeline::VulkanGraphicsPipeline::CreationInput graphicsPipelineCreateInfo = {
                getGraphicsPipelineNameString(createInfo),
                util::UniqueIdentifier(),
                device,
                getPipelineLayout(createInfo),
                getRenderingFlags(createInfo),
                getPipelineShaderStageCreateInfos(createInfo),
                getPipelineVertexInputStateCreateInfo(createInfo),
                getPipelineInputAssemblyStateCreateInfo(createInfo),
                getPipelineTessellationStateCreateInfo(createInfo),
                getPipelineViewportStateCreateInfo(createInfo),
                getPipelineRasterizationStateCreateInfo(createInfo),
                getPipelineMultisampleStateCreateInfo(createInfo),
                getPipelineDepthStencilStateCreateInfo(createInfo),
                getPipelineColorBlendStateCreateInfo(createInfo),
                getPipelineDynamicStateCreateInfo(createInfo),
                pipelineRenderingCreateInfo};
        graphicsPipeline = std::make_shared<pipeline::VulkanGraphicsPipeline>(graphicsPipelineCreateInfo);
    }

    std::string
    VulkanDynamicRenderPass::getGraphicsPipelineNameString(
            const VulkanDynamicRenderPass::CreationInput &createInfo) {
        return "dynamicRenderPass" + createInfo.name + "GraphicsPipeline";
    }

    VkRenderingFlags
    VulkanDynamicRenderPass::getRenderingFlags(const VulkanDynamicRenderPass::CreationInput &createInfo) {
        return 0;
    }

    uint32_t VulkanDynamicRenderPass::getPipelineViewMask(const VulkanDynamicRenderPass::CreationInput &info) {
        // i believe this is specifically for MULTIVIEW stuff, which we won't worry about initially
        return 0;
    }

    std::vector<VkFormat> VulkanDynamicRenderPass::getColorAttachmentFormats(
            const VulkanDynamicRenderPass::CreationInput &info) {
        std::vector<VkFormat> colorAttachmentFormats = {};


        return colorAttachmentFormats;
    }

    VkFormat VulkanDynamicRenderPass::getDepthAttachmentFormat(const VulkanDynamicRenderPass::CreationInput &info) {
        return VK_FORMAT_R32_UINT;
    }

    VkFormat VulkanDynamicRenderPass::getStencilAttachmentFormat(const VulkanDynamicRenderPass::CreationInput &info) {
        return VK_FORMAT_R32_UINT;
    }


    std::vector<VkPipelineShaderStageCreateInfo> VulkanDynamicRenderPass::getPipelineShaderStageCreateInfos(
            const VulkanDynamicRenderPass::CreationInput &info) {
        std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages = {};


        return pipelineShaderStages;
    }

    VkPipelineVertexInputStateCreateInfo VulkanDynamicRenderPass::getPipelineVertexInputStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                NULL,
                0,

        };
    }


    VkPipelineInputAssemblyStateCreateInfo VulkanDynamicRenderPass::getPipelineInputAssemblyStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }


    VkPipelineTessellationStateCreateInfo VulkanDynamicRenderPass::getPipelineTessellationStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }


    VkPipelineViewportStateCreateInfo VulkanDynamicRenderPass::getPipelineViewportStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }

    VkPipelineRasterizationStateCreateInfo VulkanDynamicRenderPass::getPipelineRasterizationStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }

    VkPipelineMultisampleStateCreateInfo VulkanDynamicRenderPass::getPipelineMultisampleStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }

    VkPipelineDepthStencilStateCreateInfo VulkanDynamicRenderPass::getPipelineDepthStencilStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }

    VkPipelineColorBlendStateCreateInfo VulkanDynamicRenderPass::getPipelineColorBlendStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                NULL,
                0,
        };
    }


    VkPipelineDynamicStateCreateInfo VulkanDynamicRenderPass::getPipelineDynamicStateCreateInfo(
            const VulkanDynamicRenderPass::CreationInput &info) {
        return {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, NULL, 0};
    }

    VkPipelineLayout VulkanDynamicRenderPass::getPipelineLayout(const VulkanDynamicRenderPass::CreationInput &info) {
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                               nullptr,
                                                               0,
                                                               static_cast<uint32_t>( info.descriptorSetLayouts.size()),
                                                               info.descriptorSetLayouts.data(),
                                                               static_cast<uint32_t>( info.pushConstantRanges.size()),
                                                               info.pushConstantRanges.data()};

        VkResult createPipelineLayoutResult =
                vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
        if (createPipelineLayoutResult != VK_SUCCESS) {
            throw std::runtime_error("Unable to create pipeline layout!");
        }

        return pipelineLayout;
    }


}// namespace PGraphics