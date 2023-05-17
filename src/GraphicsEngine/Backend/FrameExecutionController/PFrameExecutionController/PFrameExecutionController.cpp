//
// Created by paull on 2022-06-19.
//

#include "PFrameExecutionController.hpp"

#include <algorithm>

namespace PGraphics {

    PFrameExecutionController::PFrameExecutionController(const CreationInput &createInfo)
            : numBufferedFrames(createInfo.numBufferedFrames),
              currentFrameIndex(0),
              applicationContext(createInfo.applicationContext),
              scene(createInfo.scene),
              threadPool(createInfo.threadPool),
              allocator(VK_NULL_HANDLE),
              acquireSwapchainImageFence(VK_NULL_HANDLE) {
        initializeVmaAllocator();
        initializeSwapchainImageAcquisitionFence();

        initializeFrames();
    }

    void PFrameExecutionController::initializeFrames() {
        for (unsigned int frameIndex = 0; frameIndex < numBufferedFrames; frameIndex++) {
            PFrame::CreationInput frameCreationInput{
                    frameIndex,
                    applicationContext->getLogicalDevice(),
                    threadPool,
                    {},
                    nullptr,
                    applicationContext->getGraphicsQueueFamilyIndex(),
                    VK_NULL_HANDLE,
                    applicationContext->getGraphicsQueue()
            };

            auto frame = std::make_shared<PFrame>(frameCreationInput);
        }
    }

    void PFrameExecutionController::initializeSwapchainImageAcquisitionFence() {
        acquireSwapchainImageFence = VK_NULL_HANDLE;

        VkFenceCreateInfo fenceCreateInfo = {
                VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                nullptr,
                0
        };
        if (vkCreateFence(applicationContext->getLogicalDevice(), &fenceCreateInfo, nullptr,
                          &acquireSwapchainImageFence) != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during PFrameExecutionController creation - Unable to create swapchain image fence!");
        }
    }

    void PFrameExecutionController::executeNextFrame() {
        updateActiveBufferedFrameIndex();

        // in this new update, most of the frame execution stuff should take place within the Frame implementation
        frames[currentFrameIndex].execute();
    }

    void PFrameExecutionController::updateActiveBufferedFrameIndex() {
        auto result = vkAcquireNextImageKHR(applicationContext->getLogicalDevice(), windowSystem->getSwapchain(),
                                            UINT64_MAX,
                                            VK_NULL_HANDLE, acquireSwapchainImageFence, &currentFrameIndex);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during PFrameExecutionController::updateActiveBufferedFrameIndex - Unable to acquire next swapchain image!");
        }

        result = vkWaitForFences(applicationContext->getLogicalDevice(), 1, &acquireSwapchainImageFence, VK_FALSE,
                                 UINT64_MAX);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during PFrameExecutionController::updateActiveBufferedFrameIndex - Waiting for swapchain image fence failed!");
        }
    }


}// namespace PGraphics