//
// Created by paull on 2024-06-01.
//
#include "VulkanRenderer.hpp"
#include "../PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp"
#include "Frame.hpp"

using namespace pEngine::girEngine::backend;

vulkan::VulkanRenderer::~VulkanRenderer() = default;

void vulkan::VulkanRenderer::initializeRenderData(
    const std::vector<std::shared_ptr<VulkanRenderPass> > &passes,
    const VulkanResourceRepository *repository) {
    renderPasses = passes;

    // pre-allocate the semaphores that are used to signal that each pass' rendering commands
    // can begin after all resources have been updated
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    for (auto pass: renderPasses) {
        perPassCopySemaphores.push_back(VK_NULL_HANDLE);
        const auto result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(),
                                              &semaphoreCreateInfo, nullptr,
                                              &perPassCopySemaphores.back());
        if (result != VK_SUCCESS) {
            // TODO -> log!
        }
    }
    // we also have to set up our per-frame per-pass semaphores for sync
    for (auto &frame: frames) {
        const auto result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(),
                                              &semaphoreCreateInfo, nullptr,
                                              &frame->renderCompleteSemaphore);
        if (result != VK_SUCCESS) {
            // TODO -> log!
        }
    }

    // FOR NOW: we'll just perform all static geometry copies here; this will be moved elsewhere for render graph rewrite
    copyRenderPassStaticResourcesToGPU(repository);
}

vulkan::VulkanRenderer::VulkanRenderer(const CreationInput &createInfo)
    : guiThreadIsEnabled(createInfo.guiThreadEnabled),
      applicationContext(createInfo.applicationContext),
      presentationEngine(&createInfo.presentationEngine),
      renderPasses({}),
      allocator(createInfo.allocator),
      commandPool(VK_NULL_HANDLE),
      clearColor(createInfo.clearColor) {
    // create the renderer's own command pool
    const VkCommandPoolCreateInfo commandPoolCreateInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        applicationContext->getGraphicsQueueFamilyIndex()
    };
    auto result = vkCreateCommandPool(applicationContext->getLogicalDevice()->getVkDevice(),
                                      &commandPoolCreateInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        // TODO - log!
    }

    VkSemaphoreCreateInfo semaphore_create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };


    for (uint32_t i = 0; i < createInfo.numberOfFramesInFlight; i++) {
        // create a particular image acquired semaphore for one of the FIFs (note: these are passed into the frame
        // at render time after acquiring)
        imageAcquiredSemaphores.emplace_back(
            VK_NULL_HANDLE
        );
        result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(), &semaphore_create_info,
                                   nullptr, &imageAcquiredSemaphores.back());
        if (result != VK_SUCCESS) {
            // TODO - log!
        }

        VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(),
                                   &semaphoreCreateInfo,
                                   nullptr,
                                   &renderCompleteSemaphore);
        if (result != VK_SUCCESS) {
            // TODO - log!
        }
        VkSemaphore initialTransitionSemaphore = VK_NULL_HANDLE;
        result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(), &semaphoreCreateInfo, nullptr,
                                   &initialTransitionSemaphore);
        if (result != VK_SUCCESS) {
            // TODO -> log!
        }
        VkCommandPool frameCommandPool = VK_NULL_HANDLE;
        result = vkCreateCommandPool(applicationContext->getLogicalDevice()->getVkDevice(), &commandPoolCreateInfo,
                                     nullptr, &frameCommandPool);

        frames.push_back(
            std::make_unique<Frame>(std::move(Frame{
                    .index = i,
                    .fence = VK_NULL_HANDLE,
                    .swapchainImage = presentationEngine->getSwapchain().getSwapchainImage(i),
                    .imageAcquiredSemaphore = VK_NULL_HANDLE,
                    .renderCompleteSemaphore = renderCompleteSemaphore,
                    .swapchainImageViews = {presentationEngine->getSwapchain().getSwapchainImageView(i)},
                    .commandPool = frameCommandPool,
                })
            ));

        VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        result = vkCreateFence(applicationContext->getLogicalDevice()->getVkDevice(), &fenceCreateInfo, nullptr,
                               &frames.back()->fence);
        if (result != VK_SUCCESS) {
            // TODO -> log!
        }
    }

    if (guiThreadIsEnabled) {
        guiThread = std::make_unique<gui::VulkanGuiThread>(
            *this,
            *applicationContext,
            *presentationEngine,
            createInfo.guiThreadCallbacks
        );
    }
}

void vulkan::VulkanRenderer::recordEndOfFrameSwapchainImageTransition(VkCommandBuffer &endOfFrameCommandBuffer,
                                                                      const VkImageMemoryBarrier2 &
                                                                      finalSwapchainImageTransitionBarrier) const {
    endOfFrameCommandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo endOfFrameCommandBufferInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };
    auto result = vkAllocateCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(),
                                           &endOfFrameCommandBufferInfo, &endOfFrameCommandBuffer);
    if (result != VK_SUCCESS) {
        // TODO -> log!
        return;
    }
    VkCommandBufferBeginInfo endOfFrameCommandBufferBeginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr
    };
    result = vkBeginCommandBuffer(endOfFrameCommandBuffer, &endOfFrameCommandBufferBeginInfo);
    if (result != VK_SUCCESS) {
        // TODO -> log!
        return;
    }
    recordCommandBufferWithImageMemoryBarrier(endOfFrameCommandBuffer, finalSwapchainImageTransitionBarrier);
    vkEndCommandBuffer(endOfFrameCommandBuffer);
}

VkFence vulkan::VulkanRenderer::getPreviousFrameFence() const {
    return frames[previousFrameIndex]->fence;
}

VkFence vulkan::VulkanRenderer::getCurrentFrameFence() const {
    return frames[currentFrameIndex]->fence;
}

void vulkan::VulkanRenderer::recordInitialImageTransitionCommands(
    VkCommandBuffer initialImageTransitionCommandBuffer) const {
    const VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    auto result = vkBeginCommandBuffer(initialImageTransitionCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        // TODO -> log!
        return;
    }

    // first transition swapchain image into color attachment layout
    VkImageMemoryBarrier2 initialSwapchainTransitionBarrier = {};
    prepareAcquiredSwapchainImageInitialTransitionBarrier(
        frames[currentFrameIndex]->swapchainImage,
        initialSwapchainTransitionBarrier);
    recordCommandBufferWithImageMemoryBarrier(initialImageTransitionCommandBuffer,
                                              initialSwapchainTransitionBarrier);

    // then transition any depth image attachments
    VkImageMemoryBarrier2 depthImageTransitionBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        // TODO -> ensure these stage/access masks are okay
        .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = VK_NULL_HANDLE,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    for (auto &renderPass: renderPasses) {
        auto depth_image = ((VulkanDynamicRenderPass *) renderPass.get())->depthImage;
        if (depth_image != VK_NULL_HANDLE) {
            depthImageTransitionBarrier.image = depth_image;
            recordCommandBufferWithImageMemoryBarrier(initialImageTransitionCommandBuffer, depthImageTransitionBarrier);
        }
    }
    vkEndCommandBuffer(initialImageTransitionCommandBuffer);
}

void vulkan::VulkanRenderer::freePreviouslyUsedCommandBuffers(const VkCommandPool commandPool) const {
    const std::vector<VkCommandBuffer> allocatedFrameCommandBuffers = frames[currentFrameIndex]->
            allocatedCommandBuffers;
    if (!allocatedFrameCommandBuffers.empty()) {
        vkFreeCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(), commandPool,
                             allocatedFrameCommandBuffers.size(), allocatedFrameCommandBuffers.data());
        frames[currentFrameIndex]->allocatedCommandBuffers.clear();
    }
    vkResetCommandPool(applicationContext->getLogicalDevice()->getVkDevice(), commandPool,
                       VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    guiThread->resetCommandPool(currentFrameIndex);
}

/**
 * This is still split-up very haphazardly between the renderer and the frame, a giant cleanup will be good to
 * make sure everything is logically consistent and easy to follow
 * @return a set of semaphores for presentation to wait for
 */
std::vector<VkSemaphore> vulkan::VulkanRenderer::constructAndSubmitRenderCommands() {
    VkCommandPool commandPool = frames[currentFrameIndex]->commandPool;
    freePreviouslyUsedCommandBuffers(commandPool);

    // 1. trigger gui draw command recording (new design should use futures to return a submission that will be hooked in)
    // NOTE: this functionality is disabled for the interim release
    // auto futureGuiDrawCommandBuffer = guiThread->recordDrawCommands(*frames[currentFrameIndex]);

    // 2. record commands for per-frame updates (which can stay as per-render-pass for now, so this step is skipped)
    // TODO

    // 3.a -> record initial image transition commands
    VkCommandBufferAllocateInfo singlePrimaryCommandBufferAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer frameInitCommandBuffer = VK_NULL_HANDLE;
    allocateAndRecordFrameInitCommandBuffer(frameInitCommandBuffer);
    frames[currentFrameIndex]->allocatedCommandBuffers.emplace_back(frameInitCommandBuffer);

    VkCommandBufferSubmitInfo frameInitCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = frameInitCommandBuffer,
    };

    // 3.b -> record commands for drawing, obtain set of final wait semaphores that gui commands will wait on
    VkCommandBuffer frameRenderCommands = VK_NULL_HANDLE;
    auto result = vkAllocateCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(),
                                           &singlePrimaryCommandBufferAllocInfo, &frameRenderCommands);
    if (result != VK_SUCCESS) {
        // TODO -> log!
    }
    frames[currentFrameIndex]->allocatedCommandBuffers.emplace_back(frameRenderCommands);

    VkCommandBufferBeginInfo frameRenderCommandsBeginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr
    };
    result = vkBeginCommandBuffer(frameRenderCommands, &frameRenderCommandsBeginInfo);
    if (result != VK_SUCCESS) {
        // TODO -> log!
    }
    std::vector<VkCommandBuffer> renderPassCopyCommands = {};

    for (auto &pass: renderPasses) {
        auto &renderPass = *std::dynamic_pointer_cast<VulkanDynamicRenderPass>(pass);
        std::vector<VkCommandBuffer> thisPassCopyCommands = {};
        const auto renderResult = renderPass.recordRenderingCommands(frameRenderCommands,
                                                                     currentFrameIndex,
                                                                     thisPassCopyCommands,
                                                                     commandPool);
        if (renderResult != VulkanRenderPass::RenderResult::SUCCESS) {
            // TODO - log!
        }


        // store copy commands
        for (auto &buffer: thisPassCopyCommands) {
            renderPassCopyCommands.push_back(buffer);
            frames[currentFrameIndex]->allocatedCommandBuffers.push_back(buffer);
        }
    }

    // 4.a -> record final swapchain transition barrier onto gui command buffer, which is hardcoded as last to execute
    VkImageMemoryBarrier2 finalSwapchainTransitionBarrier = {};
    prepareAcquiredSwapchainImageFinalTransitionBarrier(
        frames[currentFrameIndex]->swapchainImage,
        finalSwapchainTransitionBarrier);

    // 4.b -> wait on gui command buffer future, record final swapchain image transition
    recordRenderGraphGuiPipelineBarrierCommandBuffer(frameRenderCommands);
    guiThread->recordDrawCommandsOnCurrentThread(*frames[currentFrameIndex], frameRenderCommands);
    recordCommandBufferWithImageMemoryBarrier(frameRenderCommands, finalSwapchainTransitionBarrier);

    // 4.c -> construct render commands submission
    std::vector<VkCommandBufferSubmitInfo> initSubmitCommandBuffers = {};
    // 4.c.i -> add all per-pass copy commands before the frame init command buffer (which has a barrier)
    // NEW: all copy commands will be done before the frame init barrier since they should be done first
    for (auto &buffer: renderPassCopyCommands) {
        initSubmitCommandBuffers.push_back({
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = buffer,
        });
    }
    initSubmitCommandBuffers.push_back(frameInitCommandBufferSubmitInfo);

    vkEndCommandBuffer(frameRenderCommands);
    VkCommandBufferSubmitInfo frameRenderCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = frameRenderCommands,
    };

    // 5 -> submit
    VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkSemaphore frameInitCompleteSemaphore = VK_NULL_HANDLE;
    result = vkCreateSemaphore(applicationContext->getLogicalDevice()->getVkDevice(), &semaphoreCreateInfo,
                               nullptr, &frameInitCompleteSemaphore);
    VkSemaphoreSubmitInfo frameInitSemaphoreSignalSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frameInitCompleteSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
    };

    // 5a. -> construct submission with all pre-rendering commands that must be waited for
    VkSubmitInfo2 frameInitSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = static_cast<uint32_t>(initSubmitCommandBuffers.size()),
        .pCommandBufferInfos = initSubmitCommandBuffers.data(),
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &frameInitSemaphoreSignalSubmitInfo,
    };

    VkSemaphoreSubmitInfo frameInitSemaphoreWaitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frameInitCompleteSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
    };

    VkSemaphoreSubmitInfo renderWaitSemaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frames[currentFrameIndex]->imageAcquiredSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    std::vector renderWaitSemaphoreInfos = {
        renderWaitSemaphoreInfo,
        frameInitSemaphoreWaitInfo,
    };

    std::vector renderSignalSemaphoreInfos = {
        VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            // construct gui command signal semaphore info, which I think will just tie into our particular frame for now
            .semaphore = frames[currentFrameIndex]->renderCompleteSemaphore,
            // this is the SRC scope for a signal operation
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        }
    };
    // 5b. -> construct submission containing all rendering commands
    VkSubmitInfo2 frameRenderSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = static_cast<uint32_t>(renderWaitSemaphoreInfos.size()),
        .pWaitSemaphoreInfos = renderWaitSemaphoreInfos.data(),
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &frameRenderCommandBufferSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<uint32_t>(renderSignalSemaphoreInfos.size()),
        .pSignalSemaphoreInfos = renderSignalSemaphoreInfos.data(),
    };

    // submit everything for this frame
    std::vector frameSubmits = {
        frameInitSubmitInfo,
        frameRenderSubmitInfo,
    };
    result = vkQueueSubmit2(applicationContext->getGraphicsQueue(),
                            static_cast<uint32_t>(frameSubmits.size()),
                            frameSubmits.data(),
                            frames[currentFrameIndex]->fence
    );
    if (result != VK_SUCCESS) {
        // TODO -> log!
    }

    // 6. lastly, return the list of wait semaphores for our *last* submission(s) (chronologically)
    std::vector<VkSemaphore> returnSemaphores = {};
    for (auto &info: renderSignalSemaphoreInfos) {
        returnSemaphores.push_back(info.semaphore);
    }
    return returnSemaphores;
}

void vulkan::VulkanRenderer::allocateAndRecordFrameInitCommandBuffer(VkCommandBuffer &frameInitCommandBuffer) const {
    frameInitCommandBuffer = VK_NULL_HANDLE;
    const VkCommandBufferAllocateInfo frameInitBufferAllocateInfo{

        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        frames[currentFrameIndex]->commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };
    if (vkAllocateCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(),
                                 &frameInitBufferAllocateInfo,
                                 &frameInitCommandBuffer) != VK_SUCCESS) {
        // TODO -> log!
        return;
    }

    VkImageMemoryBarrier2 initialSwapchainImageTransitionBarrier = {};
    prepareAcquiredSwapchainImageInitialTransitionBarrier(frames[currentFrameIndex]->swapchainImage,
                                                          initialSwapchainImageTransitionBarrier);

    // begin the frame-init command buffer (for initial copies)
    VkCommandBufferBeginInfo beginInfo{
        // does this really work as a constexpr?
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr
    };
    auto result = vkBeginCommandBuffer(frameInitCommandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        // TODO - log!
        return;
    }

    // 2b. queue up an additional command that simply clears the image
    // first we transition from undefined into general (for clearing) (consider transfer dst optimal instead)
    recordCommandBufferWithImageMemoryBarrier(frameInitCommandBuffer, initialSwapchainImageTransitionBarrier);
    // second we clear the image after it has been transitioned to general layout
    recordCommandBufferWithClearCommand(frameInitCommandBuffer, frames[currentFrameIndex]->swapchainImage);
    // third we transition the cleared image from general into color attachment optimal
    recordFrameInitBarriers(frameInitCommandBuffer, frames[currentFrameIndex]->swapchainImage);

    // it's a little weird/janky because of the current temporary/hacky rendering code flow, but
    // I think it's just easiest to pull and record image barriers for the depth attachments only once
    // and so we'll shove it in the frameInitCommandBuffer
    VkImageMemoryBarrier2 depthImageTransitionBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = VK_NULL_HANDLE,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        }
    };
    for (auto &renderPass: renderPasses) {
        if (((VulkanDynamicRenderPass *) renderPass.get())->depthImage != VK_NULL_HANDLE) {
            depthImageTransitionBarrier.image = ((VulkanDynamicRenderPass *) renderPass.get())->depthImage;
            recordCommandBufferWithImageMemoryBarrier(frameInitCommandBuffer, depthImageTransitionBarrier);
        }
    }

    result = vkEndCommandBuffer(frameInitCommandBuffer);
    if (result != VK_SUCCESS) {
        // TODO -> log!
    }
}

auto vulkan::VulkanRenderer::renderFrame() -> RenderFrameResult {
    // FIRST: unset all the image-acquired semaphores;
    // we have a ring of semaphores that we cycle through,
    // so we don't have to know which specific swapchain index was acquired
    for (const auto &frame: frames) {
        frame->imageAcquiredSemaphore = VK_NULL_HANDLE;
    }
    currentImageAcquiredSemaphore = (currentImageAcquiredSemaphore + 1) % imageAcquiredSemaphores.size();
    const auto currentFrameInFlightImageAcquiredSemaphore = imageAcquiredSemaphores[currentImageAcquiredSemaphore];

    // SECOND: acquire the image (and therefore determine which frame in flight gets used)
    unsigned nextImageIndex = 0u;
    auto result = vkAcquireNextImageKHR(
        applicationContext->getLogicalDevice()->getVkDevice(),
        presentationEngine->getSwapchain().getHandle(),
        UINT64_MAX,
        currentFrameInFlightImageAcquiredSemaphore,
        VK_NULL_HANDLE,
        &nextImageIndex
    );
    if (result != VK_SUCCESS) {
        // TODO - log!
        if (result == VK_NOT_READY) {
        }

        return RenderFrameResult::FAILURE;
    }

    // THIRD: update current frame index, so we know which FIF to use
    currentFrameIndex = nextImageIndex;
    if (currentFrameIndex == previousFrameIndex && frameCount != 0) {
        // TODO -> log! ideally we don't want this to happen much
        return RenderFrameResult::FAILURE;
    }
    // NEW: after we acquire the current frame index, we know which frame needs to have its "image acquired" sem set
    frames[currentFrameIndex]->imageAcquiredSemaphore = currentFrameInFlightImageAcquiredSemaphore;

    // FOURTH -> trigger frame rendering
    if (!frames[currentFrameIndex]->render(*this)) {
        // TODO -> log!
        return RenderFrameResult::FAILURE;
    }
    previousFrameIndex = currentFrameIndex;

    frameCount++;
    return RenderFrameResult::SUCCESS;
}

void
vulkan::VulkanRenderer::recordCommandBufferWithClearCommand(const VkCommandBuffer &buffer,
                                                            VkImage swapchainImage) const {
    constexpr VkImageSubresourceRange clearImageSubresourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        VK_REMAINING_MIP_LEVELS,
        0,
        VK_REMAINING_ARRAY_LAYERS
    };
    vkCmdClearColorImage(buffer, swapchainImage, VK_IMAGE_LAYOUT_GENERAL,
                         reinterpret_cast<const VkClearColorValue *>(clearColor.data()), 1,
                         &clearImageSubresourceRange);
}

void vulkan::VulkanRenderer::recordRenderGraphGuiPipelineBarrierCommandBuffer(VkCommandBuffer &buffer) {
    if (buffer == VK_NULL_HANDLE) {
        //TODO -> log!
    }
    VkMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        // block frag shader from happening at all
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        // block all memory reads until color attachment output has completed
        .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
    };

    VkImageMemoryBarrier2 swapchainImageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = frames[currentFrameIndex]->swapchainImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo renderGraphGuiBarrierInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &barrier,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &swapchainImageBarrier,
    };

    vkCmdPipelineBarrier2(buffer, &renderGraphGuiBarrierInfo);
}

void
vulkan::VulkanRenderer::recordFrameInitBarriers(const VkCommandBuffer &buffer,
                                                const VkImage &swapchainImage) const {
    const VkImageMemoryBarrier2 transitionBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        nullptr,
        // NEW: force execution barrier where we wait for clear to finish (specific stages are rec'd by spec i think)
        // make all color attachment writes (namely clearing) available for this image
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
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

    std::vector<VkBufferMemoryBarrier2> bufferBarriers = {};
    for (auto &renderPass: renderPasses) {
        auto dynamicPass = std::dynamic_pointer_cast<VulkanDynamicRenderPass>(renderPass);
        // for each pass we need to take any dynamic uniform buffers and make sure they have a memory barrier
        for (auto &dynamicBuffer: dynamicPass->dynamicUniformBuffers) {
            VulkanBufferSuballocationHandle bufferHandle = {};
            for (auto &boundBuffer: dynamicPass->boundUniformBuffers) {
                if (boundBuffer.suballocation.resourceId == dynamicBuffer.resourceId) {
                    bufferHandle = boundBuffer;
                    break;
                }
            }

            bufferBarriers.push_back(VkBufferMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                // make all writes available? maybe can target this better
                .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = bufferHandle.parentBuffer,
                .offset = bufferHandle.suballocation.globalBufferOffset,
                .size = bufferHandle.suballocation.size
            });
        }
    }

    const VkDependencyInfo dependencyInfo{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        nullptr,
        0,
        0,
        nullptr,
        static_cast<uint32_t>(bufferBarriers.size()),
        bufferBarriers.data(),
        1,
        &transitionBarrier,
    };

    vkCmdPipelineBarrier2(buffer, &dependencyInfo);
}
