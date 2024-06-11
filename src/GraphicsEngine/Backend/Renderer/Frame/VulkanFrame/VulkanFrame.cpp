//
// Created by paull on 2022-12-01.
//

#include "VulkanFrame.hpp"

namespace pEngine::girEngine::backend::frameController::frame::vulkan {

    static const VkPipelineStageFlagBits DEFAULT_SUBMISSION_WAIT_PIPELINE_STAGE_FLAGS = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

    VulkanFrame::VulkanFrame(const VulkanFrame::CreationInput &creationInput) : frameIndex(creationInput.frameIndex),
//                                                                                threadPool(creationInput.threadPool),
                                                                                renderPassesToExecute(
                                                                                        creationInput.renderPassesToExecute),
                                                                                logicalDevice(
                                                                                        creationInput.logicalDevice),
                                                                                globalResourceDescriptorSetLayout(
                                                                                        creationInput.globalResourceDescriptorSetLayout),
                                                                                workSubmissionQueue(
                                                                                        creationInput.workSubmissionQueue),
                                                                                presentQueue(
                                                                                        creationInput.workSubmissionQueue),
                                                                                workSubmissionQueueFamilyIndex(
                                                                                        creationInput.queueFamilyIndex) {
        perFrameCommandPool = VK_NULL_HANDLE;
        drawSubmissionSemaphore = VK_NULL_HANDLE;

        initializePerFrameCommandPool(creationInput);

        initializeDrawSubmissionSemaphore();

        initializeWorkerThreadVulkanDataArray(creationInput);
    }

    void VulkanFrame::initializePerFrameCommandPool(const VulkanFrame::CreationInput &creationInput) {
        VkCommandPoolCreateInfo perFrameCommandPoolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                nullptr,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                creationInput.queueFamilyIndex
        };

        auto result = vkCreateCommandPool(creationInput.logicalDevice, &perFrameCommandPoolCreateInfo, nullptr,
                                          &perFrameCommandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error during VulkanFrame creation: unable to create per-frame command pool!");
        }
    }

    void VulkanFrame::initializeDrawSubmissionSemaphore() {
        VkSemaphoreCreateInfo drawSubmissionSemaphoreCreateInfo = {
                VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                nullptr,
                0
        };

        auto result = vkCreateSemaphore(logicalDevice, &drawSubmissionSemaphoreCreateInfo, nullptr,
                                        &drawSubmissionSemaphore);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                    std::string("Error during VulkanFrame creation: Unable to create draw submission semaphore!"));
        }
    }

    void VulkanFrame::initializeWorkerThreadVulkanDataArray(const VulkanFrame::CreationInput &creationInput) {
//        perThreadVulkanDataArray.resize(threadPool->getNumberOfWorkerThreads());
        for (auto &workerThreadData: perThreadVulkanDataArray) {
            workerThreadData = std::make_shared<PerThreadVulkanData>();

            initializePerThreadCommandPool(creationInput, *workerThreadData);

            workerThreadData->renderCommandBuffer = VK_NULL_HANDLE;
        }
    }

    void VulkanFrame::initializePerThreadCommandPool(const VulkanFrame::CreationInput &creationInput,
                                                     VulkanFrame::PerThreadVulkanData &workerThreadData) {

        VkCommandPoolCreateInfo perThreadCommandPoolCreateInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                nullptr,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                creationInput.queueFamilyIndex
        };

        auto result = vkCreateCommandPool(logicalDevice, &perThreadCommandPoolCreateInfo, nullptr,
                                          &workerThreadData.commandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Error during VulkanFrame creation: Unable to create per frame command pool!");
        }
    }

    void VulkanFrame::execute() {
        for (auto &renderPass: renderPassesToExecute) {
//            renderPass->recordRenderPassDrawCommandsIntoFrame(*this);
        }

        // HARDCODED - dear imgui pass goes after all other rendering has completed
//        dearImguiRenderPass->recordRenderPassDrawCommandsIntoFrame(*this);

        std::vector<VkCommandBuffer> workerThreadCommandBuffers = gatherWorkerThreadCommandBuffers();
        transitionSwapchainImageToPresentLayout(workerThreadCommandBuffers.back());
        submitWorkerThreadCommandBuffers(workerThreadCommandBuffers);

        presentRenderedSwapchainImage();
    }

//    std::vector<VkCommandBuffer>
//    VulkanFrame::gatherWorkerThreadCommandBuffers() {
//        std::vector<VkCommandBuffer> workerThreadCommandBuffers = {};
//        for (int workerThreadIndex = 0;
//             workerThreadIndex < threadPool->getNumberOfWorkerThreads(); workerThreadIndex++)
//        {
//            std::unique_lock<std::mutex> uniqueLock(perThreadVulkanDataArray[workerThreadIndex]->lock);
//            workerThreadCommandBuffers.push_back(perThreadVulkanDataArray[workerThreadIndex]->renderCommandBuffer);
//        }
//        return workerThreadCommandBuffers;
//    }

    void VulkanFrame::transitionSwapchainImageToPresentLayout(VkCommandBuffer commandBuffer) {
        auto finalCommandBuffer = commandBuffer;
        recordPipelineBarrierCommandForSwapchainImageLayoutTransition(finalCommandBuffer);
    }

    void VulkanFrame::submitWorkerThreadCommandBuffers(std::vector<VkCommandBuffer> &workerThreadCommandBuffers) const {
        VkPipelineStageFlags pipelineWaitFlags = DEFAULT_SUBMISSION_WAIT_PIPELINE_STAGE_FLAGS;
        VkSubmitInfo frameSubmitInfo = {
                VK_STRUCTURE_TYPE_SUBMIT_INFO,
                nullptr,
                0,
                nullptr,
                &pipelineWaitFlags,
                static_cast<uint32_t>(workerThreadCommandBuffers.size()),
                workerThreadCommandBuffers.data(),
                1,
                &drawSubmissionSemaphore
        };
        auto result = vkQueueSubmit(workSubmissionQueue, 1, &frameSubmitInfo, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error in VulkanFrame::submitWorkerThreadCommandBuffers() - unable to perform queue submission!");
        }
    }

    void VulkanFrame::presentRenderedSwapchainImage() const {
        VkPresentInfoKHR presentInfo = {
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                nullptr,
                1,
                &drawSubmissionSemaphore,
                1,
                reinterpret_cast<VkSwapchainKHR const *>(windowSystem->getSwapchain()),
                &frameIndex,
                nullptr
        };

        auto result = vkQueuePresentKHR(presentQueue, &presentInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during VulkanFrame::presentRenderedSwapchainImage() - unable to present frame!");
        }
    }

    void VulkanFrame::recordPipelineBarrierCommandForSwapchainImageLayoutTransition(
            VkCommandBuffer commandBufferForRecordingCommand) {
        VkImageMemoryBarrier swapchainImageMemoryBarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                nullptr,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                0,
                0,
                windowSystem->getSwapchainImage(frameIndex),
                {
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        0,
                        1,
                        0,
                        1,
                }};

        vkCmdPipelineBarrier(commandBufferForRecordingCommand,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &swapchainImageMemoryBarrier);
    }

    void VulkanFrame::waitForPreviousFramePresentationToFinish() {
        // deprecated I think - the image will have been acquired with 'vkAcquireNextImageKHR` so we know presentation
        // is completed
    }

} // PGraphics