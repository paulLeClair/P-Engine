#pragma once

#include "../PRender.hpp"
#include "../../../vulkan_memory_allocator/vk_mem_alloc.h"

#include "../../utils/UniqueID.hpp"

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace backend {
    class Buffer;
    class Context;
    class FrameContext;

    // gonna do something weird: implement a backend class here,
    // since it's only used by the RenderGraph but i wanna delineate it somewhat
    struct GeometryContext {
      VkCommandPool copyCommandPool;
    };
};

class RenderGraph;

namespace scene {

class Renderable;
class Mesh;
class PatchList;
class Material;

struct SceneCreateInfo {
  std::shared_ptr<ThreadPool> threadPool;
  std::shared_ptr<backend::Context> context;
  std::shared_ptr<RenderGraph> renderGraph;
  VmaAllocator allocator;
};

class Scene {
  public:
    Scene(const SceneCreateInfo &info);
    ~Scene() = default;

    void registerMesh(const std::string &name, std::shared_ptr<Mesh> mesh);
    void deleteMesh(const std::string &name); 

    void registerPatchList(const std::string &name, std::shared_ptr<PatchList> patchList);
    void deletePatchList(const std::string &name);

    const VkBuffer &getHostSceneBuffer() {
      return sceneBuffer_;
    }

    const VkBuffer &getGPUSceneBuffer() {
      return gpuBuffer_;
    }

    unsigned long long getSceneBufferSizeInBytes() {
      return sceneBufferSizeInBytes_;
    }

    // bake!
    void bake();

    void update(backend::FrameContext &frame);

  private:
    // scene state...
    std::shared_ptr<ThreadPool> threadPool_;

    std::shared_ptr<backend::Context> context_;

    std::shared_ptr<RenderGraph> renderGraph_;

    VmaAllocator allocator_;

    // I think we might need to maintain some backend-specific stuff for the graph, to facilitate
    // execution with the Scene geometry
    std::shared_ptr<backend::GeometryContext> geometryContext_;

    /* SCENE STATE */
    bool isBaked_ = false;

    // in our case, 

    /* GEOMETRY */
    // all renderables should boil down to some geometry data that will be aggregated... 
      // not entirely sure how i want the scene to store these, but maybe we can just store em in a big array
      // of renderables and identify them by a name string (not sure tbh)
    std::vector<std::shared_ptr<Renderable>> meshes_ = {}; // storing them as renderables might enable ignoring the particular Vertex template argument used
    std::map<std::string, unsigned int> meshNames_ = {}; 
    std::map<UniqueID::uid_t, unsigned int> meshIDs_ = {}; 

    std::vector<std::shared_ptr<Renderable>> patchLists_ = {};
    std::map<std::string, unsigned int> patchListNames_ = {};
    std::map<UniqueID::uid_t, unsigned int> patchListIDs_ = {}; 

    // any other scene geometry subcomponents...
    
    /* BACKEND */
      // here i'll keep all the stuff involved with integration with the (Vulkan) graphics backend
  
    // we'll use a single big scene buffer and copy all geometry into an assigned subset
    VkBuffer sceneBuffer_ = VK_NULL_HANDLE; // this is basically the host-facing buffer that will be copied to VRAM at the start of renderFrame()
    VmaAllocation sceneBufferAllocation_;
    unsigned long long sceneBufferSizeInBytes_ = 0;

    VkBuffer gpuBuffer_ = VK_NULL_HANDLE;
    VmaAllocation gpuBufferAllocation_;

    struct SceneBufferSubset {
        unsigned int startByte = ~0u;
        unsigned int subsetSizeInBytes = ~0u;
    };
    std::map<UniqueID::uid_t, SceneBufferSubset> sceneBufferSubsets_ = {};
    
    std::shared_ptr<JobQueue> sceneUpdateQueue_;

    /* MISC FUNCTIONS */
    std::shared_ptr<Renderable> findRenderableByUID(UniqueID::uid_t uid);

    void copyHostSceneBufferToGPU(VkCommandBuffer &copyBuffer);

    void copyUpdatesToHostSceneBuffer(const std::vector<std::pair<std::shared_ptr<SceneBufferSubset>, UniqueID::uid_t>> &updates);

    void fillSceneBufferSingleThread();

    void fillSceneBuffer() {
      // todo: multithreaded logic
    }
};

}