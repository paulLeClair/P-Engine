//
// Created by paull on 2025-05-23.
//

#pragma once

#include <vulkan/vulkan.h>

#include "VulkanRenderer.hpp"

namespace pEngine::girEngine::backend::vulkan {
    struct Frame {
        // i need to think about how to actually hook in the per-frame copies; probably makes sense to obtain
        // it directly from the renderer inside of the frame's render() function itself

        const uint32_t index;

        VkFence fence;

        VkImage swapchainImage;

        // NOTE: this semaphore is re-set every frame using the VulkanRenderer's ring of image acquired semaphores
        VkSemaphore imageAcquiredSemaphore;

        /**
         * This is used to signal that the entirety of rendering commands for this frame has completed, and presentation
         * may proceed.
         *
         */
        VkSemaphore renderCompleteSemaphore;

        std::vector<VkImageView> swapchainImageViews = {};

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> allocatedCommandBuffers;

        /**
         * Not sure if I'll actually refactor/redesign the render per-frame-update code to live in this new
         * Frame object for *this* time around, but definitely it's on the TODO list.
         *
         * For the time being maybe I'll just leave the shell of this here
         * @return
         */
        bool updateResources() {
            return true;
        }

        /**
         * NOTE: this is called *after* we've acquired a swapchain image already,
         * so renderer.currentFrameIndex and renderer.previousFrameIndex should be initialized,
         * and we should also have that this->index == renderer.currentFrameIndex. I might modify these assumptions though,
         * and have the swapchain image acquire (and therefore index updates) occur here (but that seems weird)
         *
         * @param renderer Current renderer being used
         * @return Success or failure of frame rendering.
         */
        bool render(VulkanRenderer &renderer) {
            // idea here: this will wrap up the existing render code (which will internally be modified a bit too)
            // so that we can centralize the whole per-frame shebang in this class

            // 1. obtain current frame fence and wait on it
            bool waitResult = waitForFrameFence(renderer);
            // bool waitResult = true; // DEBUG -> disabling fence waiting because we're linearizing the frames for testing
            if (!waitResult) return false;

            // 2. trigger rendering command recording and submission (including per-frame updates and gui render commands)
            // NOTE -> this function also needs to trigger recording of gui commands and include them in this list of submit

            // EXPERIMENT: trigger new imgui frame here (doesn't seem to fix anything though)
            renderer.newImguiFrame(); // disable when gui is not wanted

            auto renderCompleteSemaphores = renderer.constructAndSubmitRenderCommands();

            auto swapchainHandle = renderer.getPresentationEngine().getSwapchain().getHandle();
            uint32_t current_frame_index = renderer.getCurrentFrameIndex();
            VkPresentInfoKHR presentInfo{
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = static_cast<uint32_t>(renderCompleteSemaphores.size()),
                .pWaitSemaphores = renderCompleteSemaphores.data(),
                .swapchainCount = 1,
                .pSwapchains = &swapchainHandle,
                .pImageIndices = &current_frame_index,
            };

            return vkQueuePresentKHR(renderer.getApplicationContext()->getGraphicsQueue(), &presentInfo) == VK_SUCCESS;
        }

    private:
        static bool waitForFrameFence(const VulkanRenderer &renderer) {
            const VkFence fence = renderer.getCurrentFrameFence();
            if (renderer.frameCount < renderer.frames.size()) {
                // skip waiting for initial frames where fences have not been set up for signaling
                return true;
            }
            auto result = vkWaitForFences(renderer.getApplicationContext()->getLogicalDevice()->getVkDevice(),
                                          1, &fence, VK_TRUE, UINT64_MAX);
            if (result != VK_SUCCESS) {
                // TODO -> log!
                return false;
            }

            result = vkResetFences(renderer.getApplicationContext()->getLogicalDevice()->getVkDevice(), 1,
                                   &fence);
            return result == VK_SUCCESS;
        }
    };
}
