#pragma once

#include "../../PRender.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <deque>
#include <functional>

class PEngine;

namespace backend {
  class Context;
  class FrameContext;
}

class RenderGraph;
class VulkanGUIData;

class ImGuiContext;
class ImGuiIO;

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

    // this a function that handles failed vulkan API calls within the imGUI lib
    // void (*CheckVkResultFn)(VkResult err);

    std::shared_ptr<backend::Context> context; // might want to make this not a context
    PEngine *engineCore = nullptr;
};

class VulkanGUIHandler {
  public:
    VulkanGUIHandler(VulkanIMGUICreateInfo &createInfo);
    ~VulkanGUIHandler();

    void registerGUIComponent(std::function<void()> imguiFunction);
    
    void clearGUIComponents() {
        imguiDrawCalls_.clear();
    }

    // CORE MENU
    void drawCoreMenu();

    void renderFrame(backend::FrameContext &frameContext);
    void presentFrame();

  private:
    std::unique_ptr<VulkanGUIData> guiData_;

    // needs a pointer to the vulkan backend wrapper which it shouldn't use much
    std::shared_ptr<backend::Context> context_;

    PEngine *engineCore_;
    
    std::deque<std::function<void()>> imguiDrawCalls_;

};


