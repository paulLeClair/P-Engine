//
// Created by paull on 2022-11-14.
//

#include "DearImguiVulkanRenderPass.hpp"

#include "../../../FrameExecutionController/Frame/PFrame/PFrame.hpp"
#include "../../../../../lib/dear_imgui/imgui_impl_vulkan.h"
#include "../../../../../lib/dear_imgui/imgui_impl_win32.h"

#include <stdexcept>

namespace PGraphics {

    const std::vector<VkDescriptorPoolSize> DearImguiVulkanRenderPass::DESCRIPTOR_POOL_SIZES = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                100},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          100},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          100},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   100},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   100},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         100},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         100},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       100}
    };

    DearImguiVulkanRenderPass::DearImguiVulkanRenderPass(
            const DearImguiVulkanRenderPass::CreationInput &creationInput) : context(ImGui::GetCurrentContext()),
                                                                             instance(creationInput.instance),
                                                                             physicalDevice(
                                                                                     creationInput.physicalDevice),
                                                                             logicalDevice(
                                                                                     creationInput.logicalDevice),
                                                                             graphicsAndPresentQueue(
                                                                                     creationInput.graphicsAndPresentQueue),
                                                                             graphicsAndPresentQueueFamilyIndex(
                                                                                     creationInput.graphicsAndPresentQueueFamilyIndex),
                                                                             guiRenderableCallbacks(
                                                                                     creationInput.initialGuiRenderableCallbacks),
                                                                             swapchainImageSize(
                                                                                     creationInput.swapchainImageSize) {

        ImGui::SetCurrentContext(context);
        initializeDearImguiVulkanImplementation(creationInput);

        initializeVulkanDescriptorPool(creationInput.numberOfSwapchainImages);
        initializeVulkanCommandPool(creationInput);
        initializeDearImGuiCommandBuffers(creationInput);

        initializeVulkanRenderPass(creationInput.swapchainImageFormat);
        initializeVulkanFramebuffers(creationInput);

        initializeImmediateSubmissionFence();
        initializeDearImguiFontsTexture();
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

    void DearImguiVulkanRenderPass::initializeDearImguiVulkanImplementation(
            const DearImguiVulkanRenderPass::CreationInput &creationInput) {
        ImGui_ImplVulkan_InitInfo imguiInitInfo = {};
        imguiInitInfo.Instance = instance;
        imguiInitInfo.Device = logicalDevice;
        imguiInitInfo.PhysicalDevice = physicalDevice;
        imguiInitInfo.ImageCount = creationInput.numberOfSwapchainImages;
        imguiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imguiInitInfo.Queue = graphicsAndPresentQueue;
        imguiInitInfo.QueueFamily = graphicsAndPresentQueueFamilyIndex;
        imguiInitInfo.DescriptorPool = guiDescriptorPool;
        imguiInitInfo.Subpass = 0;
        imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
        imguiInitInfo.MinImageCount = creationInput.numberOfSwapchainImages;
        imguiInitInfo.Allocator = nullptr;
        imguiInitInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&imguiInitInfo, guiRenderPass);
    }

    void DearImguiVulkanRenderPass::initializeVulkanDescriptorPool(unsigned int numberOfSwapchainImages) {
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.pNext = nullptr;
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(DESCRIPTOR_POOL_SIZES.size());
        descriptorPoolCreateInfo.pPoolSizes = DESCRIPTOR_POOL_SIZES.data();
        descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(numberOfSwapchainImages);

        auto result = vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo, nullptr, &guiDescriptorPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create GUI descriptor pool!");
        }
    }

    void
    DearImguiVulkanRenderPass::initializeVulkanCommandPool(
            const DearImguiVulkanRenderPass::CreationInput &creationInput) {
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
    DearImguiVulkanRenderPass::initializeDearImGuiCommandBuffers(
            const DearImguiVulkanRenderPass::CreationInput &creationInput) {
        guiCommandBuffers.resize(creationInput.numberOfSwapchainImages);

        VkCommandBufferAllocateInfo guiAllocInfo = {};
        guiAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        guiAllocInfo.pNext = nullptr;
        guiAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        guiAllocInfo.commandPool = guiCommandPool;
        guiAllocInfo.commandBufferCount = static_cast<uint32_t>(guiCommandBuffers.size());

        auto result = vkAllocateCommandBuffers(logicalDevice, &guiAllocInfo, guiCommandBuffers.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to create GUI handler command buffers!");
        }
    }

    void DearImguiVulkanRenderPass::initializeVulkanRenderPass(VkFormat swapchainImageFormat) {
        VkAttachmentDescription attachmentDescription = {};
        attachmentDescription.flags = 0;
        attachmentDescription.format = swapchainImageFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachmentReference = {};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentReference;

        VkSubpassDependency subpassDependency;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0; // we only use one subpass
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0; // apparently could also set this to VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &guiRenderPass) != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during DearImguiVulkanRenderPass creation: Unable to create IMGUI Render Pass!");
        }
    }

    void
    DearImguiVulkanRenderPass::initializeVulkanFramebuffers(
            const DearImguiVulkanRenderPass::CreationInput &creationInput) {
        std::vector<VkImageView> attachment(1);
        VkFramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.pNext = nullptr;
        fbCreateInfo.renderPass = guiRenderPass;
        fbCreateInfo.attachmentCount = 1;
        fbCreateInfo.pAttachments = attachment.data(); // set it to the temp var
        fbCreateInfo.width = creationInput.swapchainImageSize.width;
        fbCreateInfo.height = creationInput.swapchainImageSize.height;
        fbCreateInfo.layers = 1;

        auto numImages = creationInput.numberOfSwapchainImages;
        std::vector<VkImageView> swapchainImageViews = creationInput.swapchainImageViews;
        guiFramebuffers.resize(numImages);
        for (uint32_t i = 0; i < guiFramebuffers.size(); i++) {
            attachment[0] = swapchainImageViews[i];
            auto result = vkCreateFramebuffer(logicalDevice, &fbCreateInfo, nullptr, &guiFramebuffers[i]);
            if (result != VK_SUCCESS) {
                throw std::runtime_error(
                        "Error in DearImguiVulkanRenderPass creation: Unable to create imgui framebuffer!");
            }
        }
    }

    void DearImguiVulkanRenderPass::initializeDearImguiFontsTexture() {
        immediatelySubmitCommand([&](VkCommandBuffer cmd) {
                                     if (!ImGui_ImplVulkan_CreateFontsTexture(cmd)) {
                                         throw std::runtime_error(
                                                 "Error in DearImguiVulkanRenderPass creation: Unable to create ImGui Vulkan fonts texture!");
                                     }
                                 }
        );

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    /**
     * WARNING: VERY SLOW! This functionality should probably be moved out of this class, but since I don't have any other
     * places where I want to be submitting commands like this I'll just leave it in this class for now
     *
     * @param command
     */
    void
    DearImguiVulkanRenderPass::immediatelySubmitCommand(const std::function<void(VkCommandBuffer)> &command) {

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

        vkWaitForFences(logicalDevice, 1, &guiCommandPoolImmediateSubmissionFence, true, 9999999);
        vkResetFences(logicalDevice, 1, &guiCommandPoolImmediateSubmissionFence);

        vkResetCommandPool(logicalDevice, guiImmediateSubmissionCommandPool, 0);
        vkDestroyCommandPool(logicalDevice, guiImmediateSubmissionCommandPool, nullptr);
    }

    void
    DearImguiVulkanRenderPass::recordRenderPassDrawCommandsIntoFrame(const PFrame &frame) {
        if (guiRenderableCallbacks.empty()) {
            return;
        }
        auto currentFrameIndex = frame.getFrameIndex();

        beginNewImguiFrame();

        setupImguiRenderables();

        beginCommandBufferForCurrentFrame(currentFrameIndex);
        beginRenderPassForCurrentFrame(currentFrameIndex);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), guiCommandBuffers[currentFrameIndex]);

        endRenderPassForCurrentFrame(currentFrameIndex);
        endCommandBufferForCurrentFrame(currentFrameIndex);
    }

    void DearImguiVulkanRenderPass::endCommandBufferForCurrentFrame(unsigned int currentFrameIndex) {
        auto result = vkEndCommandBuffer(guiCommandBuffers[currentFrameIndex]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error during DearImguiVulkanRenderPass::draw: Unable to end command buffer!");
        }
    }

    void DearImguiVulkanRenderPass::endRenderPassForCurrentFrame(
            unsigned int currentFrameIndex) { vkCmdEndRenderPass(guiCommandBuffers[currentFrameIndex]); }

    void DearImguiVulkanRenderPass::setupImguiRenderables() {
        executeDearImGuiCallbacksToDrawRenderables();
        renderAndResetCommandPool();
    }

    void DearImguiVulkanRenderPass::beginRenderPassForCurrentFrame(unsigned int currentFrameIndex) {
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = guiRenderPass;
        renderPassBeginInfo.renderArea.extent = swapchainImageSize;
        renderPassBeginInfo.renderArea.offset = {};
        renderPassBeginInfo.clearValueCount = 0;
        renderPassBeginInfo.pClearValues = nullptr;
        renderPassBeginInfo.framebuffer = guiFramebuffers[currentFrameIndex];

        vkCmdBeginRenderPass(guiCommandBuffers[currentFrameIndex], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);
    }

    void DearImguiVulkanRenderPass::beginCommandBufferForCurrentFrame(unsigned int currentFrameIndex) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        auto result = vkBeginCommandBuffer(guiCommandBuffers[currentFrameIndex], &beginInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to begin gui command buffer!");
        }
    }

    void DearImguiVulkanRenderPass::renderAndResetCommandPool() const {
        ImGui::Render();

        vkResetCommandPool(logicalDevice, guiCommandPool, 0);
    }

    void DearImguiVulkanRenderPass::executeDearImGuiCallbacksToDrawRenderables() {
        for (auto it = guiRenderableCallbacks.begin(); it != guiRenderableCallbacks.end(); it++) {
            (*it)();
        }
    }

    void DearImguiVulkanRenderPass::beginNewImguiFrame() const {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

} // PGraphics