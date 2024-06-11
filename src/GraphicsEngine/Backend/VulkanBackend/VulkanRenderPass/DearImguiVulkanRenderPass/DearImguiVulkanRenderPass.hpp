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
#include "../../../ApplicationContext/OSInterface/OSInterface.hpp"
#include "../../../../../lib/dear_imgui/imgui.h"

#include "../../../../../lib/dear_imgui/imgui_impl_vulkan.h"
#include "../../../../../lib/dear_imgui/imgui_impl_win32.h"

namespace pEngine::girEngine::backend::vulkan {

    /**
     * This class is handled differently from other VulkanRenderPass subclasses - it contains mostly ancient code
     * that I'll likely have to reconfigure a bit, but it's also a special case where we're using an external library
     * that has its own requirements for Vulkan boilerplate and what not. \n\n
     *
     * In the end, it'll probably be a little janky but is mainly supposed to be a sort of "baseline" GUI render pass
     * for debugging, prototyping, etc - fancier GUI stuff is probably better handled by setting up a custom render
     * pass (with whatever shaders/resources are required) for fancier GUI stuff. That said, DearImGui is super handy
     * and I'll likely use it for a ton of "production" GUI stuff because I don't want to spend a lot of time trying to
     * make my own ultra-fancy GUI implementation until I learn more and attempting to make my own becomes
     * less daunting of a task. \n\n
     *
     * COMING BACK TO THIS:\n\n
     * 1. the swapchain shit is going to be ported out of this class and into the new swapchain stuff; actually
     * now that I look at it more closely, the swapchain stuff is all just being handed in so we can just use this
     * to inform what we need to be passing in from the new swapchain class\n
     * 2. this (and probably all classes in the RenderPass hierarchy) will need to be modified to work with the forthcoming
     * present mode shite; not entirely sure what that'll look like off the bat but I'll return to it once swapchains are somewhat fleshed out
     *
     *
     */
    class DearImguiVulkanRenderPass : public VulkanRenderPass {
    public:
        struct CreationInput {
            VkInstance instance;
            VkPhysicalDevice physicalDevice;
            VkDevice logicalDevice;

            VkRenderPass guiRenderPass; // temporary solution: pass in an externally-created render pass (init with public static function)

            unsigned int numberOfSwapchainImages;
            VkFormat swapchainImageFormat;
            VkExtent2D swapchainImageSize;

            // TODO - figure out whether this needs to respect the chosen present mode; might require some new abstractions
            // but hopefully i can make it so this interface can be used by any present mode and that part is assumed
            // to be managed externally
            VkQueue graphicsAndPresentQueue;
            unsigned int graphicsAndPresentQueueFamilyIndex;
            std::vector<VkImageView> swapchainImageViews;

            std::vector<std::function<void()>> initialGuiRenderableCallbacks = {};

#ifdef _WIN32
            HWND hwnd;
#endif
        };

        explicit DearImguiVulkanRenderPass(const CreationInput &creationInput);

        ~DearImguiVulkanRenderPass() = default;

        [[nodiscard]] bool isDearImguiRenderPass() const override; // TODO - rip out these stupid "is*()" methods

        // TODO -> FIX THIS PRESSING ISSUE (delete this comment when fixed):
        // I think the imgui stuff is memory leaking, the performance tanks after not that long..
        // it's likely I'm just missing an API call somewhere
        void recordDearImguiCommandBuffers(VkCommandBuffer &frameCommandBuffer, unsigned currentFrameIndex) {
            if (guiRenderableCallbacks.empty()) {
                return;
            }
            beginNewImguiFrame();

            setupImguiRenderables();

            beginRenderPassForCurrentFrame(frameCommandBuffer, currentFrameIndex);

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameCommandBuffer);

            vkCmdEndRenderPass(frameCommandBuffer);
        }

        [[nodiscard]] const VkCommandBuffer &getImguiCommandBuffer(const unsigned bufferIndex) const {
            return guiCommandBuffers[bufferIndex];
        }

        static bool
        initializeVulkanRenderPass(const VkDevice &device, VkFormat swapchainImageFormat,
                                   VkRenderPass &guiRenderPass);

    private:
        static const std::vector<VkDescriptorPoolSize> DESCRIPTOR_POOL_SIZES;

        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;

        VkExtent2D swapchainImageSize;
        unsigned numberOfSwapchainImages;

        VkQueue graphicsAndPresentQueue;
        unsigned int graphicsAndPresentQueueFamilyIndex;

        ImGuiContext *context;
        ImGuiIO &io = ImGui::GetIO();

        VkDescriptorPool guiDescriptorPool;

        VkCommandPool guiCommandPool;
        VkFence guiCommandPoolImmediateSubmissionFence;

        std::vector<VkCommandBuffer> guiCommandBuffers = {};

        VkRenderPass guiRenderPass;
        std::vector<VkFramebuffer> guiFramebuffers = {};

        std::vector<std::function<void()>> guiRenderableCallbacks = {};

        void initializeVulkanDescriptorPool(unsigned int swapchainImages);

        void initializeDearImguiVulkanImplementation(const CreationInput &creationInput);

        void initializeVulkanFramebuffers(const CreationInput &creationInput);

        void initializeVulkanCommandPool(const CreationInput &creationInput);

        void initializeDearImGuiCommandBuffers();

        void initializeImmediateSubmissionFence();

        void initializeDearImguiFontsTexture();

        void immediatelySubmitCommand(const std::function<void(VkCommandBuffer)> &command);

        void beginNewImguiFrame() const;

        void executeDearImGuiCallbacksToDrawRenderables();

        void beginCommandBufferForCurrentFrame(unsigned int currentFrameIndex);

        void beginRenderPassForCurrentFrame(VkCommandBuffer &commandBuffer, unsigned int currentFrameIndex);

        void setupImguiRenderables();

        void endCommandBufferForCurrentFrame(unsigned int currentFrameIndex);
    };

} // PGraphics
