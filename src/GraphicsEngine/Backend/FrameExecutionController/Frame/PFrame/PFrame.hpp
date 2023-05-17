//
// Created by paull on 2022-12-01.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include "../../../../../EngineCore/ThreadPool/PThreadPool/PThreadPool.hpp"
#include "../Frame.hpp"
#include "../../../PBackend/VulkanRenderPass/VulkanRenderPass.hpp"

#include "../../../ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.hpp"
#include "../../../PBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp"

#include <vector>
#include <memory>

namespace PGraphics {

    class PFrame : public Frame {
    public:
        struct PerThreadVulkanData {
            std::mutex lock = std::mutex();

            VkDevice logicalDevice = VK_NULL_HANDLE;

            VkCommandPool commandPool = VK_NULL_HANDLE;
            VkCommandBuffer renderCommandBuffer = VK_NULL_HANDLE;

            VkDescriptorSet globalResourceDescriptorSet = VK_NULL_HANDLE;
        };

        struct CreationInput {
            unsigned int frameIndex;

            VkDevice logicalDevice;

            std::shared_ptr<PEngine::PThreadPool> threadPool;

            std::vector<std::shared_ptr<VulkanRenderPass>> renderPassesToExecute;

            std::shared_ptr<DearImguiVulkanRenderPass> dearImguiVulkanRenderPass;

            unsigned int queueFamilyIndex;

            VkDescriptorSetLayout globalResourceDescriptorSetLayout;

            VkQueue workSubmissionQueue;
        };

        explicit PFrame(const CreationInput &creationInput);

        ~PFrame() = default;

        void execute() override;

        [[nodiscard]] unsigned int getFrameIndex() const override {
            return frameIndex;
        }

        PerThreadVulkanData &getPerThreadVulkanData(unsigned int threadIndex) {
            if (threadIndex > threadPool->getNumberOfWorkerThreads()) {
                throw std::runtime_error("Error in PFrame::getPerThreadVulkanData: invalid thread index!");
            }
            return *perThreadVulkanDataArray[threadIndex];
        }

    private:
        unsigned int frameIndex;

        std::shared_ptr<PEngine::PThreadPool> threadPool;

        std::shared_ptr<PWindowSystem> windowSystem;

        VkDevice logicalDevice;

        VkCommandPool perFrameCommandPool;

        VkDescriptorSetLayout globalResourceDescriptorSetLayout;

        /**
         * This is the semaphore that is provided as a signal semaphore for queue submission, and then also is
         * used as a wait semaphore for queue presentation at the end of frame execution
         */
        VkSemaphore drawSubmissionSemaphore;

        std::vector<std::shared_ptr<PerThreadVulkanData>> perThreadVulkanDataArray = {};

        std::vector<std::shared_ptr<VulkanRenderPass>> renderPassesToExecute;
        std::shared_ptr<DearImguiVulkanRenderPass> dearImguiRenderPass;

        unsigned int workSubmissionQueueFamilyIndex;

        VkQueue workSubmissionQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;

        void initializePerFrameCommandPool(const CreationInput &creationInput);

        void initializeDrawSubmissionSemaphore();

        void initializePerThreadCommandPool(const CreationInput &creationInput, PerThreadVulkanData &workerThreadData);

        void initializeWorkerThreadVulkanDataArray(const CreationInput &creationInput);

        void waitForPreviousFramePresentationToFinish();

        void
        recordPipelineBarrierCommandForSwapchainImageLayoutTransition(VkCommandBuffer commandBufferForRecordingCommand);

        void transitionSwapchainImageToPresentLayout(VkCommandBuffer commandBuffer);

        std::vector<VkCommandBuffer> gatherWorkerThreadCommandBuffers();

        void submitWorkerThreadCommandBuffers(std::vector<VkCommandBuffer> &workerThreadCommandBuffers) const;

        void presentRenderedSwapchainImage() const;

    };

} // PGraphics
