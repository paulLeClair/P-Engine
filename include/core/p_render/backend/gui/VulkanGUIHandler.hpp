#pragma once

// MAR20 - i commented this all out for now, i think this class will end up being used as the way to interact with the 
// "hardcoded" GUI pass that will be set up at the end of the render graph when the engine starts
  // to disable the GUI youll have to go out of your way, i think having it set up like this will be fine for the purposes of the engine
  // therefore: TODO -> implement high-level GUI interface

// #include "../../../gui/GUIHandler.hpp"

// include vulkan stuff
  // this is an issue
// #include "../../PRender.hpp"
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

    // MAR3 - idea: since i need to give the guihandler access to the render graph, let's just stick that in this struct:
    // std::shared_ptr<RenderGraph> renderGraph;
    // OSInterfacePtr os;
    std::shared_ptr<Backend::Context> context; // might want to make this not a context
    PEngine *engineCore = nullptr;
    // ImGuiData &data;
};

class VulkanGUIHandler /* : public GUIHandler */ {
  public:
    VulkanGUIHandler(VulkanIMGUICreateInfo &createInfo);
    ~VulkanGUIHandler();

    // eventually probably use preproc stuff to implement the correct functions, for now i'll just do VulkanInstanceData
        // maybe this should give a reference to the manager not the instance
    // void init(const VulkanInstanceData &vulkanInstance); // don't think this can use the VulkanInstanceData directly
    // void init(); 
        // might not need this with a *CreateInfo thing

    // should probably come up with some kinda baby interface for registering new imgui stuff to draw
        // APR11 - while i'm learning, i think i'll just make a few simple functions that kinda chunk together
        // large menus...
    void registerGUIComponent(std::function<void()> imguiFunction);
    
    void clearGUIComponents() {
        imguiDrawCalls_.clear();
    }

    // CORE MENU (temporary?)
    void drawCoreMenu();

    void renderFrame(Backend::FrameContext &frameContext);
    void presentFrame();

    // void createImGuiFramebuffers(Backend::ImGuiData &data);

  private:
    // MAR21 - new direction: gonna separate GUI and scene data
      // that way i don't have to do some weird thing where the scenedata object has to pass the GUIData to this GUIhandler class
    // plus it makes sense to have the GUIHandler manage the GUI data
    std::unique_ptr<VulkanGUIData> guiData_;

    // NEW ADDTITION: pointer to render graph, which it shouldn't use much
      // if this turnns out to be unnecessary i'll get rid of it
    // std::shared_ptr<RenderGraph> renderGraph_;
    // OSInterfacePtr os_;

    // needs a pointer to the vulkan backend wrapper which it shouldn't use much
    std::shared_ptr<Backend::Context> context_;

    // APR13
        // somehow need a way to trigger transitions b/w engine modes/etc here...
        // my idea is to just include a pointer to the engine and trigger it that way
    PEngine *engineCore_;
    
    // gonna maintain a simple std::deque of std::functions to call that should define all the ImGui components to be drawn
    std::deque<std::function<void()>> imguiDrawCalls_;

    // need imgui command pool and command buffers for each 
    // VkCommandPool guiCommandPool_;
    // std::vector<VkCommandBuffer> guiCommandBuffers_;

    // // framebuffers
    // std::vector<VkFramebuffer> guiFramebuffers_;


};


