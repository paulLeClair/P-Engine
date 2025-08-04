//
// Created by paull on 2022-11-14.
//

#include "DearImguiVulkanRenderPass.hpp"

#ifdef _WIN32
#include "../../../lib/dear_imgui/imgui_impl_win32.h"
#endif

#include <stdexcept>

namespace pEngine::girEngine::backend::vulkan {
    const std::vector<VkDescriptorPoolSize> DearImguiVulkanRenderPass::DESCRIPTOR_POOL_SIZES = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100}
    };

    DearImguiVulkanRenderPass::DearImguiVulkanRenderPass(
        const CreationInput &creationInput) : instance(creationInput.instance),
                                              physicalDevice(
                                                  creationInput.physicalDevice),
                                              logicalDevice(
                                                  creationInput.logicalDevice),
                                              swapchainImageSize(
                                                  creationInput.swapchainImageSize),
                                              numberOfSwapchainImages(
                                                  creationInput.numberOfSwapchainImages),
                                              graphicsAndPresentQueue(
                                                  creationInput.graphicsAndPresentQueue),
                                              graphicsAndPresentQueueFamilyIndex(
                                                  creationInput.graphicsAndPresentQueueFamilyIndex),
                                              context(ImGui::GetCurrentContext()),
                                              guiRenderableCallbacks(
                                                  creationInput.initialGuiRenderableCallbacks) {
        ImGui::SetCurrentContext(context);

        initializeVulkanDescriptorPool(creationInput.numberOfSwapchainImages);

        initializeVulkanCommandPool(creationInput);

        initializeDearImGuiCommandBuffers();

#ifdef _WIN32
        ImGui_ImplWin32_Init(creationInput.hwnd);
#endif

        VkPipelineRenderingCreateInfo guiPassDynamicPipelineRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1, // TODO -> allow multiple color attachments
            .pColorAttachmentFormats = &creationInput.swapchainImageFormat,
            // TODO -> depth for gui if needed
        };

        ImGui_ImplVulkan_InitInfo vulkanInitInfo = {
            .Instance = instance,
            .PhysicalDevice = physicalDevice,
            .Device = logicalDevice,
            .QueueFamily = graphicsAndPresentQueueFamilyIndex,
            .Queue = graphicsAndPresentQueue,
            .DescriptorPool = guiDescriptorPool,
            .MinImageCount = numberOfSwapchainImages,
            .ImageCount = numberOfSwapchainImages,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT, // TODO -> enable gui multisampling
            .UseDynamicRendering = true,
            .PipelineRenderingCreateInfo = guiPassDynamicPipelineRenderingInfo,
        };
        if (const auto vulkanInitSuccessful = ImGui_ImplVulkan_Init(&vulkanInitInfo); vulkanInitSuccessful != true) {
            //TODO-> log!
        }

        initializeImmediateSubmissionFence();

        initializeDearImguiFontsTexture();

        ImGui::StyleColorsDark();
    }

    void DearImguiVulkanRenderPass::initializeImmediateSubmissionFence() {
        VkFenceCreateInfo dearImGuiCommandPoolFenceCreateInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            0
        };

        if (vkCreateFence(logicalDevice, &dearImGuiCommandPoolFenceCreateInfo, nullptr,
                          &guiCommandPoolImmediateSubmissionFence) != VK_SUCCESS) {
            throw std::runtime_error(
                "Error in DearImguiVulkanRenderPass creation: Unable to create VkFence to be used for immediate submission!");
        }
    }

    void DearImguiVulkanRenderPass::initializeVulkanDescriptorPool(unsigned int swapchainImages) {
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.pNext = nullptr;
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(DESCRIPTOR_POOL_SIZES.size());
        descriptorPoolCreateInfo.pPoolSizes = DESCRIPTOR_POOL_SIZES.data();
        descriptorPoolCreateInfo.maxSets = swapchainImages;

        auto result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &guiDescriptorPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create GUI descriptor pool!");
        }
    }

    void
    DearImguiVulkanRenderPass::initializeVulkanCommandPool(
        const CreationInput &creationInput) {
        VkCommandPoolCreateInfo guiPoolCreateInfo = {};
        guiPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        guiPoolCreateInfo.pNext = nullptr;
        guiPoolCreateInfo.queueFamilyIndex = creationInput.graphicsAndPresentQueueFamilyIndex;
        guiPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(logicalDevice, &guiPoolCreateInfo, nullptr, &guiCommandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create GUI handler command pool!");
        }
    }

    void
    DearImguiVulkanRenderPass::initializeDearImGuiCommandBuffers() {
        // is this where we want to reset the command pool?
        auto result = vkResetCommandPool(logicalDevice, guiCommandPool, 0);
        if (result != VK_SUCCESS) {
            // TODO - log!
        }

        guiCommandBuffers.resize(numberOfSwapchainImages);
        VkCommandBufferAllocateInfo guiAllocInfo = {};
        guiAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        guiAllocInfo.pNext = nullptr;
        guiAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        guiAllocInfo.commandPool = guiCommandPool;
        guiAllocInfo.commandBufferCount = static_cast<uint32_t>(guiCommandBuffers.size());

        result = vkAllocateCommandBuffers(logicalDevice, &guiAllocInfo, guiCommandBuffers.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create GUI handler command buffers!");
        }
    }

    void DearImguiVulkanRenderPass::initializeDearImguiFontsTexture() {
        if (!ImGui_ImplVulkan_CreateFontsTexture()) {
            throw std::runtime_error(
                "Error in DearImguiVulkanRenderPass creation: Unable to create ImGui Vulkan fonts texture!");
        }

        ImGui_ImplVulkan_DestroyFontsTexture();
    }

    void
    DearImguiVulkanRenderPass::immediatelySubmitCommand(const std::function<void(VkCommandBuffer)> &command) const {
        VkCommandPool guiImmediateSubmissionCommandPool;
        VkCommandPoolCreateInfo guiImmediateSubmissionCommandPoolCreateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            nullptr,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            graphicsAndPresentQueueFamilyIndex
        };

        auto result = vkCreateCommandPool(logicalDevice, &guiImmediateSubmissionCommandPoolCreateInfo, nullptr,
                                          &guiImmediateSubmissionCommandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error during immediate command submission: Unable to create command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = guiImmediateSubmissionCommandPool;

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        result = vkAllocateCommandBuffers(logicalDevice, &allocInfo, &cmd);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to allocate command buffer from base command pool!");
        }

        VkCommandBufferBeginInfo cmdBeginInfo = {};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmdBeginInfo.pInheritanceInfo = nullptr;
        cmdBeginInfo.pNext = nullptr;

        result = vkBeginCommandBuffer(cmd, &cmdBeginInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to begin command buffer!");
        }

        command(cmd);

        result = vkEndCommandBuffer(cmd);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to end command buffer!");
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;

        result = vkQueueSubmit(graphicsAndPresentQueue, 1u, &submitInfo, guiCommandPoolImmediateSubmissionFence);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to submit to core command pool!");
        }

        vkWaitForFences(logicalDevice, 1, &guiCommandPoolImmediateSubmissionFence, true, UINT64_MAX);
        vkResetFences(logicalDevice, 1, &guiCommandPoolImmediateSubmissionFence);

        vkResetCommandPool(logicalDevice, guiImmediateSubmissionCommandPool, 0);
        vkDestroyCommandPool(logicalDevice, guiImmediateSubmissionCommandPool, nullptr);
    }

    void DearImguiVulkanRenderPass::setupImguiRenderables() {
        executeDearImGuiCallbacksToDrawRenderables();
    }

    void DearImguiVulkanRenderPass::beginRenderPassForCurrentFrame(const VkCommandBuffer &commandBuffer,
                                                                   const uint32_t currentFrameIndex) const {
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = guiRenderPass;
        renderPassBeginInfo.renderArea.extent = swapchainImageSize;
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.clearValueCount = 0;
        renderPassBeginInfo.pClearValues = nullptr;
        renderPassBeginInfo.framebuffer = guiFramebuffers[currentFrameIndex];

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void DearImguiVulkanRenderPass::executeDearImGuiCallbacksToDrawRenderables() {
        for (auto it = guiRenderableCallbacks.begin(); it != guiRenderableCallbacks.end(); it++) {
            (*it)();
        }
    }

    void DearImguiVulkanRenderPass::beginNewImguiFrame() {
        ImGui_ImplVulkan_NewFrame();
#ifdef _WIN32
        ImGui_ImplWin32_NewFrame();
#endif

        ImGui::NewFrame();
    }
} // PGraphics
