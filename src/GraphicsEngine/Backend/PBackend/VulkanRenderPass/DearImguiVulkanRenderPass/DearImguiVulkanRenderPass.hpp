//
// Created by paull on 2022-11-14.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include <functional>
#include "../VulkanRenderPass.hpp"
#include "../../../ApplicationContext/WindowSystem/WindowSystem.hpp"
#include "../../../../../lib/dear_imgui/imgui.h"

namespace PGraphics {

    /**
     * This class is handled differently from other VulkanRenderPass subclasses - only one per frame (for now)
     */
    class DearImguiVulkanRenderPass : public VulkanRenderPass {
    public:
        struct CreationInput {
            VkInstance instance;
            VkPhysicalDevice physicalDevice;
            VkDevice logicalDevice;

            unsigned int numberOfSwapchainImages;
            VkFormat swapchainImageFormat;
            VkExtent2D swapchainImageSize;

            VkQueue graphicsAndPresentQueue;
            unsigned int graphicsAndPresentQueueFamilyIndex;
            std::vector<VkImageView> swapchainImageViews;

            std::vector<std::function<void()>> initialGuiRenderableCallbacks = {};
        };

        explicit DearImguiVulkanRenderPass(const CreationInput &creationInput);

        ~DearImguiVulkanRenderPass() = default;

        void recordRenderPassDrawCommandsIntoFrame(const PFrame &frame) override;

    private:
        static const std::vector<VkDescriptorPoolSize> DESCRIPTOR_POOL_SIZES;

        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;

        VkExtent2D swapchainImageSize;

        VkQueue graphicsAndPresentQueue;
        unsigned int graphicsAndPresentQueueFamilyIndex;

        ImGuiContext *context;

        VkDescriptorPool guiDescriptorPool;

        VkCommandPool guiCommandPool;
        VkFence guiCommandPoolImmediateSubmissionFence;

        std::vector<VkCommandBuffer> guiCommandBuffers = {};

        VkRenderPass guiRenderPass;
        std::vector<VkFramebuffer> guiFramebuffers = {};

        std::vector<std::function<void()>> guiRenderableCallbacks = {};

        void initializeVulkanRenderPass(VkFormat swapchainImageFormat);

        void initializeVulkanDescriptorPool(unsigned int numberOfSwapchainImages);

        void initializeDearImguiVulkanImplementation(const CreationInput &creationInput);

        void initializeVulkanFramebuffers(const CreationInput &creationInput);

        void initializeVulkanCommandPool(const CreationInput &creationInput);

        void initializeDearImGuiCommandBuffers(const CreationInput &creationInput);

        void initializeImmediateSubmissionFence();

        void initializeDearImguiFontsTexture();

        void immediatelySubmitCommand(const std::function<void(VkCommandBuffer)> &command);

        void beginNewImguiFrame() const;

        void executeDearImGuiCallbacksToDrawRenderables();

        void renderAndResetCommandPool() const;

        void beginCommandBufferForCurrentFrame(unsigned int currentFrameIndex);

        void beginRenderPassForCurrentFrame(unsigned int currentFrameIndex);

        void setupImguiRenderables();

        void endRenderPassForCurrentFrame(unsigned int currentFrameIndex);

        void endCommandBufferForCurrentFrame(unsigned int currentFrameIndex);
    };

} // PGraphics
