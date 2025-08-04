//
// Created by paull on 2022-11-14.
//

#pragma once

#include <vulkan/vulkan_core.h>
#include <functional>

#ifdef _WIN32

#include <Windows.h>

#endif

#include "../VulkanRenderPass.hpp"
#include "../../../../../lib/dear_imgui/imgui.h"

#include "../../../../../lib/dear_imgui/imgui_impl_vulkan.h"

namespace pEngine::girEngine::backend::vulkan {
    class DearImguiVulkanRenderPass final : public VulkanRenderPass {
    public:
        struct CreationInput {
            VkInstance instance = VK_NULL_HANDLE;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            VkDevice logicalDevice = VK_NULL_HANDLE;

            unsigned int numberOfSwapchainImages = 0;
            VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
            VkExtent2D swapchainImageSize = {};

            VkQueue graphicsAndPresentQueue = VK_NULL_HANDLE;
            unsigned int graphicsAndPresentQueueFamilyIndex = 0;
            std::vector<VkImageView> swapchainImageViews = {};

#ifdef _WIN32
            HWND hwnd;
#endif

            std::vector<std::function<void()> > initialGuiRenderableCallbacks = {};
            VkRenderingInfo dynamicRenderingInfo = {};
        };

        explicit DearImguiVulkanRenderPass(const CreationInput &creationInput);

        DearImguiVulkanRenderPass() = default;

        ~DearImguiVulkanRenderPass() override = default;

        void recordDearImguiCommandBuffers(const VkCommandBuffer &frameCommandBuffer,
                                           const VkRenderingInfo &renderingInfo) {
            // run imgui callbacks and commit the rendered components
            setupImguiRenderables();
            ImGui::Render();

            // record the draw commands for the interface encoded in our callbacks
            vkCmdBeginRendering(frameCommandBuffer, &renderingInfo);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameCommandBuffer);
            vkCmdEndRendering(frameCommandBuffer);
        }

        [[nodiscard]] const VkCommandBuffer &getImguiCommandBuffer(const unsigned bufferIndex) const {
            return guiCommandBuffers[bufferIndex];
        }

        void setCallbacks(std::vector<std::function<void()> > &&callbacks) {
            guiRenderableCallbacks = std::move(callbacks);
        }

        void immediatelySubmitCommand(const std::function<void(VkCommandBuffer)> &command) const;

        static void beginNewImguiFrame();

    private:
        static const std::vector<VkDescriptorPoolSize> DESCRIPTOR_POOL_SIZES;

        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;

        VkExtent2D swapchainImageSize = {};
        unsigned numberOfSwapchainImages = 0;

        VkQueue graphicsAndPresentQueue = VK_NULL_HANDLE;
        unsigned int graphicsAndPresentQueueFamilyIndex = 0;

        ImGuiContext *context = nullptr;

        VkDescriptorPool guiDescriptorPool = VK_NULL_HANDLE;

        VkCommandPool guiCommandPool = VK_NULL_HANDLE;
        VkFence guiCommandPoolImmediateSubmissionFence = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> guiCommandBuffers = {};

        VkRenderPass guiRenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> guiFramebuffers = {};

        std::vector<std::function<void()> > guiRenderableCallbacks = {};

        void initializeVulkanDescriptorPool(unsigned int swapchainImages);


        void initializeVulkanCommandPool(const CreationInput &creationInput);

        void initializeDearImGuiCommandBuffers();

        void initializeImmediateSubmissionFence();

        static void initializeDearImguiFontsTexture();

        void executeDearImGuiCallbacksToDrawRenderables();

        void beginRenderPassForCurrentFrame(const VkCommandBuffer &commandBuffer, unsigned int currentFrameIndex) const;

        void setupImguiRenderables();
    };
} // PGraphics
