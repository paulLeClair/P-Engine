//
// Created by paull on 2025-04-30.
//

#pragma once

#include <future>
#include <mutex>
#include <semaphore>
#include <thread>

#include "../../../Backend/ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"

#include "../../../Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp"

#include "../PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp"

namespace pEngine::girEngine::backend::vulkan {
    struct Frame;
    class VulkanRenderer;
}

namespace pEngine::gui {
    // actually I'm kinda thinkin this whole gui thread thing should be removed for now;
    struct VulkanGuiThread {
        explicit VulkanGuiThread(girEngine::backend::vulkan::VulkanRenderer &parent,
                                 girEngine::backend::appContext::vulkan::VulkanApplicationContext &applicationContext,
                                 girEngine::backend::vulkan::VulkanPresentationEngine &presentationEngine,
                                 const std::vector<std::function<void()> > &callbacks);

        ~VulkanGuiThread() = default;

        void recordDrawCommandsOnCurrentThread(girEngine::backend::vulkan::Frame &frame, VkCommandBuffer &cb);

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

    private:
        girEngine::backend::vulkan::VulkanRenderer &parentRenderer;

        girEngine::backend::appContext::vulkan::VulkanApplicationContext &applicationContext;

        std::thread thread = {};

        girEngine::backend::vulkan::DearImguiVulkanRenderPass guiPass = {};

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
        void renderCommands(VkCommandBuffer guiCommandBuffer, const girEngine::backend::vulkan::Frame &frame);
    };
}
