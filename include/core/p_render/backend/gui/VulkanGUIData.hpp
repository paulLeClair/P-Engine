#pragma once

// base 
// #include "../../../scene_data/GUIData.hpp" 

// #include "./VulkanGUIHandler.hpp"

// #include "../../../../imgui/imgui.h"
// #include "../../../../imgui/imgui_internal.h"
// #include "../../../../imgui/imgui_impl_vulkan.h"
// #include "../../../../imgui/imgui_impl_win32.h"

// include vulkan stuff
// #include "../../../../../../vulkan/vulkan.h"
#include "../../PRender.hpp"

#include <unordered_map>
#include <string>
#include <stdexcept>

#include <memory>


class RenderGraph;

class Context;

// struct VulkanIMGUICreateInfo {
//     const VkInstance &vulkanInstance;
//     const VkDevice &device;
//     const VkPhysicalDevice &selectedPhysicalDevice;
//     const unsigned int &queueFamilyIndex;
//     const VkQueue &queue;


//     // const VkSampleCountFlagBits &msaaSamples;
//     // const VkAllocationCallbacks *allocator;
//     // const VkDescriptorPool &descriptorPool; 
//     // const VkPipelineCache &pipelineCache;
//     const unsigned int &minImageCount;
//     const unsigned int &imageCount;
//     // const unsigned int &subpass; 

//     // this a function that handles failed vulkan API calls within the imGUI thing i think
//     // void (*CheckVkResultFn)(VkResult err);

//     std::shared_ptr<RenderGraph> renderGraph;
// };

// imgui forward declarations
class ImGuiContext;

// struct ImGuiData {
//     ImGuiContext *ctx_;
//     ImGuiIO *io_;
//     VkDescriptorPool guiDescriptorPool;
//     VkRenderPass guiPass;

//     std::vector<VkFramebuffer> guiFramebuffers;
// };
    

class VulkanGUIData /* : public GUIData */ {
  public:
    VulkanGUIData(const VulkanIMGUICreateInfo &createInfo); // create info, no parent scene info anymore since GUI is no longer part of the scene itself
    ~VulkanGUIData();


    // getters
    const VkRenderPass &getRenderPass() const {
      return guiPass_;
    }
    
    const VkFramebuffer &getFramebuffer(uint32_t swapchainIndex) const {
      // assert(swapchainIndex < vulkan_->getImageCount());
      return frameBuffers_[swapchainIndex];
    }

    std::vector<VkCommandBuffer> &getCommandBuffers() {
      return guiCommandBuffers_;
    }

    void resetCommandPool() {
      auto result = vkResetCommandPool(context_->getLogicalDevice(), guiCommandPool_, 0);
      if (result != VK_SUCCESS) {
          throw std::runtime_error("Unable to reset Gui command pool!");
      }
    }

  private:
    // pointer to render graph 
    // std::shared_ptr<RenderGraph> renderGraph_;

    // pointer to vulkan wrapper?
    std::shared_ptr<Backend::Context> context_;

    // // vulkan/DearIMGUI stuff!
    ImGuiContext *ctx_;
    // ImGuiIO *io_;

    /* VULKAN STATE */
    // descriptor pool
    VkDescriptorPool guiDescriptorPool_;
    VkCommandPool guiCommandPool_;
    std::vector<VkCommandBuffer> guiCommandBuffers_;

    // std::unordered_map<std::string, std::shared_ptr<GUIPass>> guiPasses_;
    VkRenderPass guiPass_; // for now we just maintain a single one since that's how the imgui lib we're using works ;)

    std::vector<VkFramebuffer> frameBuffers_;


    // startup functions
    // void createDescriptorPool(); // unneeded now i think
    void setupIMGUIResources(); 

  protected:

};
