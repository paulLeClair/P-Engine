//
// Created by paull on 2025-04-30.
//

#pragma once

#include <future>
#include <mutex>
#include <semaphore>

#include "../../../Backend/VulkanBackend/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../../../Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp"


namespace pEngine::girEngine::backend::vulkan {
    class VulkanRenderer;
    struct Frame;
    class VulkanPresentationEngine;
}

namespace pEngine::girEngine::backend::vulkan {
    // actually I'm kinda thinkin this whole gui thread thing should be removed for now;
    struct VulkanGuiContext {
        VulkanGuiContext(VulkanRenderer &parent,
                         VulkanApplicationContext &applicationContext,
                         VulkanPresentationEngine &presentationEngine,
                         const std::vector<std::function<void()> > &callbacks);

        ~VulkanGuiContext() = default;

        void recordDrawCommandsOnCurrentThread(Frame &frame, VkCommandBuffer &cb);

        void setCallbacks(std::vector<std::function<void()> > &&callbacks) {
            guiPass.setCallbacks(std::move(callbacks));
        }

        [[nodiscard]] VkSemaphore getSignalSemaphore() const {
            return renderCompleteSemaphore;
        }

        void resetCommandPool(uint32_t i) {
            if (!allocatedCommandBuffers[i].empty())
                vkFreeCommandBuffers(applicationContext.getLogicalDevice()->getVkDevice(), guiThreadCommandPools[i],
                                     static_cast<uint32_t>(allocatedCommandBuffers[i].size()),
                                     allocatedCommandBuffers[i].data());
            allocatedCommandBuffers[i].clear();
            vkResetCommandPool(applicationContext.getLogicalDevice()->getVkDevice(), guiThreadCommandPools[i],
                               VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }

        void beginNewImguiFrame() const {
            guiPass.beginNewImguiFrame();
        }

        VulkanGuiContext(const VulkanGuiContext &other)
            : parentRenderer(other.parentRenderer),
              applicationContext(other.applicationContext),
              guiPass(other.guiPass),
              guiThreadCommandPools(other.guiThreadCommandPools),
              allocatedCommandBuffers(other.allocatedCommandBuffers),
              guiRenderPass(other.guiRenderPass),
              guiDrawSignalSemaphore(other.guiDrawSignalSemaphore),
              renderCompleteSemaphore(other.renderCompleteSemaphore),
              drawFrameTriggered(other.drawFrameTriggered),
              guiThreadStillAlive(other.guiThreadStillAlive),
              swapchainImageAttachmentInfos(other.swapchainImageAttachmentInfos) {
        }

        VulkanGuiContext &operator=(const VulkanGuiContext &other);

        VulkanGuiContext &operator=(VulkanGuiContext &&other) noexcept;

        std::vector<std::function<void()> > &getCallbacks() {
            return guiPass.callbacks;
        }

    private:
        VulkanRenderer &parentRenderer;

        VulkanApplicationContext &applicationContext;

        DearImguiVulkanRenderPass guiPass = {};

        std::vector<VkCommandPool> guiThreadCommandPools = {};
        // NOTE -> we may need to extend this so that it's per-frame
        std::map<uint32_t, std::vector<VkCommandBuffer> > allocatedCommandBuffers = {};

        VkRenderPass guiRenderPass;

        VkSemaphore guiDrawSignalSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;

        // synchronization primitives
        std::condition_variable guiThreadWaitConditionVariable = std::condition_variable();
        std::mutex guiThreadMutex = std::mutex();
        bool drawFrameTriggered = false;
        bool guiThreadStillAlive = true;

        std::vector<VkRenderingAttachmentInfo> swapchainImageAttachmentInfos = {};

        // NEW: this is *only* the code for recording to a command buffer that is assumed to exist
        void renderCommands(VkCommandBuffer guiCommandBuffer, const Frame &frame);
    };
}
