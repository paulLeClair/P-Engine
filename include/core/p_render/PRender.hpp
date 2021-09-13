#pragma once

#include "./backend/wsi/WindowSystem.hpp"
#include "../../core/thread_pool/job_queue/JobQueue.hpp"

// gui
#include "./backend/gui/VulkanGUIHandler.hpp"

// render graph
#include "./render_graph/RenderGraph.hpp"

// scene (TODO)
#include "./scene/Scene.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../core/p_render/backend/Context.hpp"

// VMA
#include "../../vulkan_memory_allocator/vk_mem_alloc.h"

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../../glm/glm.hpp"
#include "../../glm/gtc/quaternion.hpp"
#include "../../glm/gtc/matrix_transform.hpp"
#include "../../glm/vec3.hpp"


// VULKAN LIMITS
// should probably move these elsewhere
  // just gonna use the limits that Themaister's Granite uses, since i don't know how to judge
  // what reasonable limits are, and i also like the idea of keeping things compact
#define VULKAN_NUM_DESCRIPTOR_SETS 4
#define VULKAN_NUM_ATTACHMENTS 8
#define VULKAN_NUM_BINDINGS 32
#define VULKAN_NUM_VERTEX_ATTRIBUTES 16
#define VULKAN_NUM_VERTEX_BUFFERS 4
#define VULKAN_PUSH_CONSTANT_SIZE 128
#define VULKAN_MAX_UBO_SIZE 16 * 1024

#pragma region PRENDER_DEFINITION

class PEngine;
class VulkanGUIHandler;

namespace backend {
    class Context;
    class FrameContext;
}

namespace scene {
    class Scene;
}

class PRender {
  public:
    PRender(PEngine *engineCore);
    ~PRender();

    /* RENDER INTERFACE */
    std::shared_ptr<backend::Context> &renderContext();

    // render graph interface
    std::shared_ptr<RenderGraph> registerRenderGraph(const std::string &name);
    std::shared_ptr<RenderGraph> getRenderGraph(const std::string &name);

    VmaAllocator &getVMAllocator() {
      return allocator_;
    }

    // gui interface
    void registerGUIComponent(std::function<void()> call);
    
    void clearGUIComponents();

    /* RENDER INTERFACE */
    void renderFrame(const std::string &renderGraphName = "default");

  private:
    /* render state */
    PEngine *core_ = nullptr;

    VmaAllocator allocator_;

    /* RENDER STATE */
    
    // SCENES (TODO)

    // GRAPHS
    std::vector<std::shared_ptr<RenderGraph>> renderGraphs_;
    std::unordered_map<std::string, unsigned int> renderGraphNames_;

    // maintain a simple vector of FrameContexts, which should manage all the data for rendering one frame in one particular swapchain image
    std::vector<std::shared_ptr<backend::FrameContext>> frameContexts_;
    unsigned int activeFrameContext_;

    // gui handler
    std::shared_ptr<VulkanGUIHandler> gui_;

    /* BACKEND */
    std::shared_ptr<backend::Context> context_;

    void setupImGui();
    void setupIMGUIResources();

    void submitCommandBuffers(backend::FrameContext &frameContext);

};

#pragma endregion PRENDER_DEFINITION
