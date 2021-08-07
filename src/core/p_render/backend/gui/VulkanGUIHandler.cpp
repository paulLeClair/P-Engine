#include "../../../../../include/core/p_render/backend/gui/VulkanGUIHandler.hpp"

#include "../../../../../include/core/PEngineCore.hpp"

#include "../../../../../include/core/p_render/backend/gui/VulkanGUIData.hpp"

#include "../../../../../include/core/p_render/render_graph/RenderGraph.hpp"

#include "../../../../../include/core/p_render/backend/Context.hpp"
#include "../../../../../include/core/p_render/backend/frame_context/FrameContext.hpp"

#include "../../../../../include/core/p_render/PRender.hpp"

// imgui include
#include "../../../../../include/imgui/imgui.h"
#include "../../../../../include/imgui/imgui_internal.h"
#include "../../../../../include/imgui/imgui_impl_vulkan.h"
#include "../../../../../include/imgui/imgui_impl_win32.h"

// for now i'll just implement everything here...


VulkanGUIHandler::VulkanGUIHandler(VulkanIMGUICreateInfo &createInfo) { // for now GUIHandler has no ctor

    // i guess it needs to store a shared pointer to BOTH 
    context_ = createInfo.context;
    // renderGraph_ = createInfo.renderGraph;
    // os_ = createInfo.os;
    engineCore_ = createInfo.engineCore;

    guiData_ = std::make_unique<VulkanGUIData>(createInfo);

}

VulkanGUIHandler::~VulkanGUIHandler() { 

}

void VulkanGUIHandler::renderFrame(Backend::FrameContext &frameContext) {
    // i think this function can be submitted to a JobQueue for execution by a worker thread...
        // as far as i know this shouldn't really involve doing anything 

    // start a new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame(); // start the frame; can call ImGui functions
                        // until you call ImGui::Render()

    // go through and call all the functions for drawing imgui elements
    for (auto it = imguiDrawCalls_.begin(); it != imguiDrawCalls_.end(); it++) {
        (*it)();
    }

    // call render to conclude executing things for ImGui to draw
    ImGui::Render();

    // now we can actually render the ImGui data using vulkan
    
    // reset gui command pool
    guiData_->resetCommandPool();

    // get current swapchain index
    const uint32_t swapchainIndex = context_->getSwapchainImageIndex();
    
    // begin command buffer for current swapchain index...
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    auto result = vkBeginCommandBuffer(guiData_->getCommandBuffers()[swapchainIndex], &beginInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to begin gui command buffer!");
    }

    // begin render pass
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = guiData_->getRenderPass();
    renderPassBeginInfo.renderArea.extent = context_->getSwapchainImageSize();
    renderPassBeginInfo.renderArea.offset = {};
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = nullptr;
    renderPassBeginInfo.framebuffer = guiData_->getFramebuffer(swapchainIndex);

    vkCmdBeginRenderPass(guiData_->getCommandBuffers()[swapchainIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // record draw data into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), guiData_->getCommandBuffers()[swapchainIndex]);

    // end the command buffer and render pass
    vkCmdEndRenderPass(guiData_->getCommandBuffers()[swapchainIndex]);
    result = vkEndCommandBuffer(guiData_->getCommandBuffers()[swapchainIndex]);

    // actually we'll include the imgui cmd buffer with the rest of the frameContext's submissions 
    frameContext.enqueueCommandBuffer(guiData_->getCommandBuffers()[swapchainIndex]);
}

void VulkanGUIHandler::registerGUIComponent(std::function<void()> imguiFunction) {
    imguiDrawCalls_.push_back(imguiFunction);
}

void VulkanGUIHandler::drawCoreMenu() {
    
    ImGuiWindowFlags coreWindowFlags = 0;
    coreWindowFlags |= ImGuiWindowFlags_NoMove;
    coreWindowFlags |= ImGuiWindowFlags_NoResize;
    coreWindowFlags |= ImGuiWindowFlags_NoCollapse;
    // coreWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    
    // size window
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
    auto winSize = ImGui::GetWindowSize();
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

    ImGui::Begin("PEngine Core Menu", NULL, coreWindowFlags);
        // just gonna create an unformatted lil window for now 

    if (ImGui::Button("World Generation")) {
        // somehow signal that we entered world generation?
            // gotta think about how the engine will transition between different functionalities
        // engineCore_->enterMode("worldGen");
    }

    if (ImGui::Button("Simulate")) {
        // similarly to worldgen, somehow enter the simulation thing
        // engineCore_->enterMode("sim");
    }

    if (ImGui::Button("Options")) {
        // engineCore_->enterMode("optionsMenu");
    }

    if (ImGui::Button("Exit")) {
        // exit the whole program
        engineCore_->exit();
    }

    ImGui::End();
}