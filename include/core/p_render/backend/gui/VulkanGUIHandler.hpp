#pragma once

#include "../../PRender.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <deque>
#include <functional>

// forgot to implement this
class PEngine;

// p_render forward declarations

namespace Backend {
  class Context;
  class FrameContext;
}

class RenderGraph;
class VulkanGUIData;

// imgui forward decls
class ImGuiContext;
class ImGuiIO;
// class ImGuiData;
// honestly this is so tricky i might need to just package up the relevant data in my own *CreateInfo style struct

// some of this can be set up in the GUIHandler, which can I guess set up the render graph 
struct VulkanIMGUICreateInfo {
    VkInstance vulkanInstance;
    VkDevice device;
    VkPhysicalDevice selectedPhysicalDevice;
    unsigned int queueFamilyIndex;
    VkQueue queue;
    VkFormat swapchainImageFormat;

    // const VkSampleCountFlagBits &msaaSamples;
    // const VkAllocationCallbacks *allocator;
    // const VkDescriptorPool &descriptorPool; 
    // const VkPipelineCache &pipelineCache;
    unsigned int minImageCount;
    unsigned int imageCount;
    // const unsigned int &subpass; 

    // this a function that handles failed vulkan API calls within the imGUI thing i think
    // void (*CheckVkResultFn)(VkResult err);

    // std::shared_ptr<RenderGraph> renderGraph;
    std::shared_ptr<Backend::Context> context; // might want to make this not a context
    PEngine *engineCore = nullptr;
    // ImGuiData &data;
};

class VulkanGUIHandler /* : public GUIHandler */ {
  public:
    VulkanGUIHandler(VulkanIMGUICreateInfo &createInfo);
    ~VulkanGUIHandler();

    void registerGUIComponent(std::function<void()> imguiFunction);
    
    void clearGUIComponents() {
        imguiDrawCalls_.clear();
    }

    // CORE MENU (temporary?)
    void drawCoreMenu();

    void renderFrame(Backend::FrameContext &frameContext);
    void presentFrame();

  private:
    // MAR21 - new direction: gonna separate GUI and scene data
      // that way i don't have to do some weird thing where the scenedata object has to pass the GUIData to this GUIhandler class
    // plus it makes sense to have the GUIHandler manage the GUI data
    std::unique_ptr<VulkanGUIData> guiData_;

    // needs a pointer to the vulkan backend wrapper which it shouldn't use much
    std::shared_ptr<Backend::Context> context_;

    PEngine *engineCore_;
    
    // gonna maintain a simple std::deque of std::functions to call that should define all the ImGui components to be drawn
    std::deque<std::function<void()>> imguiDrawCalls_;

};


