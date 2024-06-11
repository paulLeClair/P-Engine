//
// Created by paull on 2024-06-01.
//
#include "VulkanRenderer.hpp"

using namespace pEngine::girEngine::backend;

render::vulkan::VulkanRenderer::RenderFrameResult render::vulkan::VulkanRenderer::renderFrame() {
    // okay I guess this is where we'll actually draw an individual frame;
    // for now I'll keep it simplified so that the renderer acquires, draws, and presents
    // entirely within this function; not sure if this will even need to change much lol

    // PROBLEM -> barriers only work within a single command buffer!
    // therefore, I'll have to refactor all this to be writing into 1 command buffer...
    // kinda lame but not that bad. In the future I can break it up so that 1 single thread
    // will do this whole process and so we'll basically have as many command buffers as threads
    // (no idea how well that'll work lmfao)

    // 1. acquire image
    boost::optional<unsigned> nextSwapchainIndex = presentationEngine->acquireNextSwapchainImageIndex();
    if (!nextSwapchainIndex.has_value()) {
        // TODO - log!
        return RenderFrameResult::FAILURE;
    }
    VkImage swapchainImage = applicationContext->getOSInterface()
            ->getSwapchain()->getSwapchainImage(nextSwapchainIndex.get());

    // maybe the problem is that we want to wait to reset the command pool until we've acquired the whole next frame
    vkResetCommandPool(applicationContext->getLogicalDevice()->getVkDevice(), commandPool,
                       0); // TODO - determine if we need flags here

    // 1a. transition swapchain to write-optimal layout
    // (I think a pipeline barrier will be sufficient for this and the other transition)
    VkImageMemoryBarrier2 initialSwapchainImageTransitionBarrier = {};
    prepareAcquiredSwapchainImageInitialTransitionBarrier(swapchainImage,
                                                          initialSwapchainImageTransitionBarrier);

    // 2a. set up list of command buffers; queue up a command buffer with swapchain image transition barrier first
    VkCommandBuffer frameCommandBuffer = VK_NULL_HANDLE;

    VkCommandBufferAllocateInfo allocateInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1
    };

    if (vkAllocateCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(), &allocateInfo,
                                 &frameCommandBuffer) != VK_SUCCESS) {
        // TODO - log!
        return RenderFrameResult::FAILURE;
    }
    VkCommandBufferBeginInfo beginInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr
    };
    auto result = vkBeginCommandBuffer(frameCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        // TODO - log!
        return RenderFrameResult::FAILURE;
    }

    recordCommandBufferWithImageMemoryBarrier(frameCommandBuffer, initialSwapchainImageTransitionBarrier);

    // 2.c queue up an additional command that simply clears the image... apparently I do have to do this myself
    // clearing keeps happening when the image is in the wrong format :(
    recordCommandBufferWithClearCommand(frameCommandBuffer, swapchainImage);
    recordTransitionSwapchainImageLayoutToColorAttachmentOptimal(frameCommandBuffer, swapchainImage);

    // 2b. run all render passes (dearimgui only for this initial pass)
    // for now, all rendering will be singlethreaded; but that will change when I hook in the thread pool utility
    // TODO - overhaul how backend objects are passed to the renderer; this will do for now
    if (dearImguiVulkanRenderPass) {
        // here's where we will have to actually run the DearImgui frame for this pass
        dearImguiVulkanRenderPass->recordDearImguiCommandBuffers(frameCommandBuffer, nextSwapchainIndex.get());
    }

    // 3a. transition swapchain image to present optimal layout
    VkImageMemoryBarrier2 finalSwapchainImageTransitionBarrier = {};
    prepareAcquiredSwapchainImageFinalTransitionBarrier(swapchainImage, finalSwapchainImageTransitionBarrier);
    recordCommandBufferWithImageMemoryBarrier(frameCommandBuffer, finalSwapchainImageTransitionBarrier);

    // end the singular command buffer; in the future our command buffer system will change drastically
    vkEndCommandBuffer(frameCommandBuffer);

    // 3b. submit command buffers
    VkSemaphoreSubmitInfo waitSemaphoreInfo{
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            nullptr,
            imageAcquiredSemaphore,
            0, // this value is ignored unless its a timeline semaphore (which we aren't using here)
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, // device group index?? i'm assuming you leave it 0 if you're not using these
    };

    VkSemaphoreSubmitInfo signalSemaphoreInfo{
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            nullptr,
            renderCompleteSemaphore,
            0,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // TODO - verify we want to use the same stage on the signal semaphore
            0
    };

    // build command buffer submit infos for the submit info 2
    VkCommandBufferSubmitInfo commandBufferSubmitInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            nullptr,
            frameCommandBuffer,
            0, // assuming we don't need to use this...?
    };

    VkSubmitInfo2 submitInfo{
            VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            nullptr,
            0,
            1,
            &waitSemaphoreInfo,
            1, // TODO - handle multithreading & multiple command buffers
            &commandBufferSubmitInfo,
            1,
            &signalSemaphoreInfo
    };
    result = vkQueueSubmit2(applicationContext->getGraphicsQueue(), 1, &submitInfo, frameFence);
    if (result != VK_SUCCESS) {
        return RenderFrameResult::FAILURE;
    }

    // wait on the frame fence
    result = vkWaitForFences(applicationContext->getLogicalDevice()->getVkDevice(), 1, &frameFence,
                             VK_TRUE, UINT64_MAX); // arbitrarily long wait
    if (result != VK_SUCCESS) {
        // TODO - log!
        return RenderFrameResult::FAILURE;
    }
    result = vkResetFences(applicationContext->getLogicalDevice()->getVkDevice(), 1, &frameFence);
    if (result != VK_SUCCESS) {
        // TODO - log!
        return RenderFrameResult::FAILURE;
    }

    // 4. present (which should hopefully be auto-sync'd with the render complete semaphore)
    auto presentedSuccessfully = presentationEngine->presentSwapchainImage(nextSwapchainIndex.get());

    return presentedSuccessfully ? RenderFrameResult::SUCCESS : RenderFrameResult::FAILURE;
}

void
render::vulkan::VulkanRenderer::recordCommandBufferWithClearCommand(VkCommandBuffer &clearSwapchainImageCommandBuffer,
                                                                    VkImage swapchainImage) {

    const
    VkImageSubresourceRange clearImageSubresourceRange{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            VK_REMAINING_MIP_LEVELS,
            0,
            VK_REMAINING_ARRAY_LAYERS
    };
    vkCmdClearColorImage(clearSwapchainImageCommandBuffer, swapchainImage, VK_IMAGE_LAYOUT_GENERAL,
                         reinterpret_cast<const VkClearColorValue *>(clearColor.data()), 1,
                         &clearImageSubresourceRange);
}

void
render::vulkan::VulkanRenderer::recordTransitionSwapchainImageLayoutToColorAttachmentOptimal(VkCommandBuffer &buffer,
                                                                                             VkImage &swapchainImage) {
    VkImageMemoryBarrier2 transitionBarrier{
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            nullptr,
            VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, // no clue if this src stage flag works...
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // this one seems to be firing off too early...
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            applicationContext->getLogicalDevice()->getGraphicsQueueFamilyIndex(),
            applicationContext->getLogicalDevice()->getGraphicsQueueFamilyIndex(),
            swapchainImage,
            {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    VK_REMAINING_MIP_LEVELS,
                    0,
                    VK_REMAINING_ARRAY_LAYERS
            }
    };

    return recordCommandBufferWithImageMemoryBarrier(buffer, transitionBarrier);
}
