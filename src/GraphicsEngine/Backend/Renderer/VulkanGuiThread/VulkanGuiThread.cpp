//
// Created by paull on 2025-05-20.
//
#include "VulkanGuiThread.hpp"

#include "../VulkanRenderer/VulkanRenderer.hpp"
#include "../VulkanRenderer/Frame.hpp"

using namespace pEngine::girEngine;

pEngine::gui::VulkanGuiThread::VulkanGuiThread(
    backend::vulkan::VulkanRenderer &parent,
    backend::appContext::vulkan::VulkanApplicationContext &
    applicationContext,
    backend::vulkan::VulkanPresentationEngine &presentationEngine,
    const std::vector<std::function<void()> > &callbacks)
    : parentRenderer(parent), applicationContext(applicationContext) {
    std::vector<VkImageView> imageViews(presentationEngine.getSwapchain().getSwapchainImages().size());
    for (uint32_t imageIndex = 0; imageIndex < imageViews.size(); imageIndex++) {
        imageViews[imageIndex] = parentRenderer.getPresentationEngine().getSwapchain().
                getSwapchainImageView(imageIndex);
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
    auto result = vkCreateSemaphore(applicationContext.getLogicalDevice()->getVkDevice(),
                                    &semaphoreCreateInfo,
                                    nullptr, &guiDrawSignalSemaphore);
    if (result != VK_SUCCESS) {
        // TODO -> log, no throw
        throw std::runtime_error("failed to create semaphore");
    }

    result = vkCreateSemaphore(applicationContext.getLogicalDevice()->getVkDevice(), &semaphoreCreateInfo,
                               nullptr, &renderCompleteSemaphore);

    if (result != VK_SUCCESS) {
        // TODO -> log, no throw
        throw std::runtime_error("failed to create semaphore");
    }

    VkCommandPoolCreateInfo poolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = applicationContext.getLogicalDevice()->getGraphicsQueueFamilyIndex(),
    };
    guiThreadCommandPools.assign(parentRenderer.frames.size(), VK_NULL_HANDLE);
    for (uint32_t i = 0; i < parentRenderer.frames.size(); i++) {
        result = vkCreateCommandPool(applicationContext.getLogicalDevice()->getVkDevice(), &poolCreateInfo,
                                     nullptr,
                                     &guiThreadCommandPools[i]);
        if (result != VK_SUCCESS) {
            // TODO -> log, no throw
            throw std::runtime_error("failed to create command pool");
        }
        allocatedCommandBuffers[i] = {};
    }

    guiPass = backend::vulkan::DearImguiVulkanRenderPass(
        backend::vulkan::DearImguiVulkanRenderPass::CreationInput{
            applicationContext.getInstance(),
            applicationContext.getPhysicalDevice(),
            applicationContext.getLogicalDevice()->getVkDevice(),
            static_cast<uint32_t>(presentationEngine.getSwapchain().getSwapchainImages().size()),
            presentationEngine.getSwapchain().swapchainImageFormat,
            presentationEngine.getSwapchain().getSwapchainImageExtent(),
            applicationContext.getGraphicsQueue(),
            applicationContext.getGraphicsQueueFamilyIndex(),
            imageViews,
#ifdef _WIN32
            applicationContext.getOSInterface()->getWin32Window()->getWinApiHWND(),
#endif
            callbacks
        });

    for (uint32_t i = 0; i < parentRenderer.frames.size(); i++) {
        const auto view = parentRenderer.getPresentationEngine().getSwapchain().getSwapchainImageView(i);
        if (view == VK_NULL_HANDLE) {
            // TODO -> log!s
            throw std::runtime_error("failed to obtain swapchain image view");
        }
        swapchainImageAttachmentInfos.push_back(VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        });
    }
}

void pEngine::gui::VulkanGuiThread::recordDrawCommandsOnCurrentThread(
    backend::vulkan::Frame &frame, VkCommandBuffer &cb) {
    // NOTE: async stuff is disabled for interim release
    
    // NEW: this just takes in an external cb and renders directly into it without async
    renderCommands(cb, frame);

    // LINEARIZATION: this will all happen on the main thread
    // const auto &recordCommandsTask = [&](VkCommandBuffer buffer) -> VkCommandBuffer {
    //     renderCommands(buffer, frame);
    //     return buffer;
    // };

    // FOR NOW -> circumventing the thread created here and not really even using it
    // return std::async(std::launch::async, recordCommandsTask, cb);
}

void pEngine::gui::VulkanGuiThread::renderCommands(VkCommandBuffer guiCommandBuffer,
                                                   const backend::vulkan::Frame &frame) {
    // 0. check command buffer handle and start recording
    if (guiCommandBuffer == VK_NULL_HANDLE) {
        // TODO -> log!
        return;
    }

    // 1. set up the dynamic rendering info we have to use the proper swapchain image view
    VkRenderingInfo dynamicRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = parentRenderer.getPresentationEngine().getDefaultSwapchainImageRenderArea(),
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &swapchainImageAttachmentInfos[frame.index],
        // TODO -> depth / stencil if applicable
    };

    // 2. trigger recording of dearimgui commands
    guiPass.recordDearImguiCommandBuffers(guiCommandBuffer, dynamicRenderingInfo);

    // 3. end recording (DISABLED FOR NOW)
    // vkEndCommandBuffer(guiCommandBuffer);
}
