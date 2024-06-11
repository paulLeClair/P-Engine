//
// Created by paull on 2022-06-19.
//

#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <memory>
#include <utility>
#include <vector>

#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../Scene/SceneResources/SceneResource.hpp"
#include "../../ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../../VulkanBackend/VulkanBuffer/VulkanBuffer.hpp"
#include "../../VulkanBackend/VulkanImage/VulkanImage.hpp"
#include "../../VulkanBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp"
#include "../../VulkanBackend/VulkanShaderModule/VulkanShaderModule.hpp"
#include "../../ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterface.hpp"
#include "../Renderer.hpp"
#include "../Frame/VulkanFrame/VulkanFrame.hpp"
#include "../PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp"

// ugly macro for now
#define DEFAULT_SWAPCHAIN_TIMEOUT_IN_MS 5

namespace pEngine::girEngine::backend::vulkan {
    class VulkanBackend;
}

namespace pEngine::girEngine::backend::render::vulkan {

    /**
     * Responsibilities of this class:
     * -> TLDR: maintain all the objects/subclasses needed for drawing a particular frame (ie rendering to a
     * render target), not including the windowing system stuff most likely
     * ->
     */
    class VulkanRenderer : public backend::render::Renderer {
    public:
        struct CreationInput {
            std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext;

            // TODO - wire in the baked backend objects; I think some kind of class that aggregates all the arrays
            // and organizes them is warranted here; it should make it easier to make efficient scene-updating code
            // that is externally triggered I hope (which of course is a much later issue)

            // in any event, even tho better ways must exist for doing this, I'll just pass in
            // the handle directly (since we're hardcoding 1 imgui render pass for the time being)

            // TODO - consider relocating the presentation engine so we dont have to configure the swapchain here
            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            std::vector<float> clearColor = {0.0, 0.0, 0.0, 0.0};

        };

        explicit VulkanRenderer(const CreationInput &createInfo)
                : applicationContext(createInfo.applicationContext),
                  dearImguiVulkanRenderPass(nullptr),
                  imageAcquiredSemaphore(VK_NULL_HANDLE),
                  renderCompleteSemaphore(VK_NULL_HANDLE),
                  frameFence(VK_NULL_HANDLE),
                  commandPool(VK_NULL_HANDLE),
                  clearColor(createInfo.clearColor) {
            initializeSynchronization();

            presentationEngine = std::make_shared<present::vulkan::VulkanPresentationEngine>(
                    present::vulkan::VulkanPresentationEngine::CreationInput{
                            applicationContext->getLogicalDevice()->getVkDevice(),
                            applicationContext->getLogicalDevice()->getGraphicsQueue(), // hardcoding graphics&present in 1 queue
                            applicationContext->getOSInterface()->getSwapchain(),
                            DEFAULT_SWAPCHAIN_TIMEOUT_IN_MS,
                            imageAcquiredSemaphore,
                            renderCompleteSemaphore
                    }
            );

            // create the renderer's own command pool
            VkCommandPoolCreateInfo commandPoolCreateInfo{
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

        }

        void initializeSynchronization() {// create semaphores
            VkSemaphoreCreateInfo semaphoreCreateInfo{
                    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                    nullptr,
                    0
            };
            auto result = vkCreateSemaphore(
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    &semaphoreCreateInfo,
                    nullptr,
                    &renderCompleteSemaphore
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
            }
            result = vkCreateSemaphore(
                    applicationContext->getLogicalDevice()->getVkDevice(),
                    &semaphoreCreateInfo,
                    nullptr,
                    &imageAcquiredSemaphore
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
            }

            VkFenceCreateInfo createInfo{
                    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    nullptr,
                    0 // create in unsignaled state
            };
            result = vkCreateFence(applicationContext->getLogicalDevice()->getVkDevice(), &createInfo, nullptr,
                                   &frameFence);
            if (result != VK_SUCCESS) {
                // TODO - log!
            }
        }

        ~VulkanRenderer() override {
            vkDestroySemaphore(applicationContext->getLogicalDevice()->getVkDevice(), imageAcquiredSemaphore, nullptr);
            vkDestroySemaphore(applicationContext->getLogicalDevice()->getVkDevice(), renderCompleteSemaphore, nullptr);
            vkDestroyFence(applicationContext->getLogicalDevice()->getVkDevice(), frameFence, nullptr);
            vkDestroyCommandPool(applicationContext->getLogicalDevice()->getVkDevice(), commandPool, nullptr);
        }

        enum class RenderFrameResult {
            SUCCESS,
            FAILURE
        };

        // temporary measure: just set the pointer externally when it's ready
        void setDearImguiVulkanRenderPassPointer(std::shared_ptr<backend::vulkan::DearImguiVulkanRenderPass> &pass) {
            dearImguiVulkanRenderPass = pass;
        }

        RenderFrameResult renderFrame();

        std::vector<float> &acquireClearColorHandle() {
            return clearColor;
        }

    private:
        std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext;

        // for the first pass I'll try to just have the renderer create the presentation engine itself
        std::shared_ptr<present::vulkan::VulkanPresentationEngine> presentationEngine;

        // TODO - figure out how best to integrate the rest of the backend baked objects; we'll need to get the passes & resources;
        // this will also factor into the update code at some point

        // TODO -> completely rework the interface between the backend's baked objects and the renderer;
        // for now, I'm going to keep it EXTRA simple and just have a hardwired single dearimgui render pass that will
        // be externally set by the backend after baking
        std::shared_ptr<::pEngine::girEngine::backend::vulkan::DearImguiVulkanRenderPass> dearImguiVulkanRenderPass = nullptr;

        // this semaphore should be used to signal when the renderer has acquired an image from the swapchain
        VkSemaphore imageAcquiredSemaphore;

        // this semaphore should be used to signal when the renderer is currently drawing to a particular swapchain image
        VkSemaphore renderCompleteSemaphore;

        // i think we need to add in another bit of synchronization: a fence.
        // in the future I can consider other synchronization methods
        VkFence frameFence; // TODO - make it so we have multiple frame fences (1 per swapchain image)

        // for simplicity, I'll have the renderer maintain its own pool for submitting certain "overhead" commands
        // that aren't part of the user's render passes
        VkCommandPool commandPool;

        // this just controls what the clear color will be before a frame is drawn
        std::vector<float> clearColor;

        static void prepareAcquiredSwapchainImageInitialTransitionBarrier(VkImage &acquiredSwapchainImage,
                                                                          VkImageMemoryBarrier2 &acquiredSwapchainImageBarrier) {
            acquiredSwapchainImageBarrier = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    nullptr,
                    VK_PIPELINE_STAGE_2_NONE, // srcStageMask
                    0, // srcAccessMask
                    VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, // dstStageMask -> don't clear until we acquire the image
                    VK_ACCESS_2_TRANSFER_WRITE_BIT, // dstAccessMask -> after we acquire it, allow clear access
                    VK_IMAGE_LAYOUT_UNDEFINED, // we don't care about whatever was in here before
                    VK_IMAGE_LAYOUT_GENERAL, // since we're clearing before any drawing happens, we want general layout
                    VK_QUEUE_FAMILY_IGNORED, // according to vulkan example, this transition does not require ownership transfer b/w queues
                    VK_QUEUE_FAMILY_IGNORED, // (which makes sense because we're discarding the contents of the image)
                    acquiredSwapchainImage,
                    {
                            VK_IMAGE_ASPECT_COLOR_BIT,
                            0,
                            VK_REMAINING_MIP_LEVELS,
                            0,
                            VK_REMAINING_ARRAY_LAYERS
                    }
            };
        }

        void
        prepareAcquiredSwapchainImageFinalTransitionBarrier(VkImage &swapchainImage, VkImageMemoryBarrier2 &barrier) {
            barrier = {
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    nullptr,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, // dstStageMask
                    0, // dstAccessFlags,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // TODO - make sure layouts line up; need to track this when we have render passes able to transition images themselves
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    applicationContext->getGraphicsQueueFamilyIndex(),
                    applicationContext->getGraphicsQueueFamilyIndex(), // TODO - present queues if needed
                    swapchainImage,
                    {
                            VK_IMAGE_ASPECT_COLOR_BIT,
                            0,
                            VK_REMAINING_MIP_LEVELS,
                            0,
                            VK_REMAINING_ARRAY_LAYERS
                    }
            };
        }

        static void
        recordCommandBufferWithImageMemoryBarrier(VkCommandBuffer &transitionBuffer,
                                                  VkImageMemoryBarrier2 imageTransitionBarrier) {

            VkDependencyInfo dependencyInfo{
                    VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    nullptr,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &imageTransitionBarrier
            };

            vkCmdPipelineBarrier2(transitionBuffer, &dependencyInfo);
        }


        void recordCommandBufferWithClearCommand(VkCommandBuffer &buffer, VkImage swapchainImage);

        void
        recordTransitionSwapchainImageLayoutToColorAttachmentOptimal(VkCommandBuffer &buffer, VkImage &swapchainImage);
    };
}
