#pragma once

#include "../Renderable.hpp"

#include "../../PRender.hpp"

// gonna try using the new utility ID class;
#include "../../../utils/UniqueID.hpp"

#include "../../../../mesh_optimizer/meshoptimizer.h"

#include <vector>
#include <memory>
#include <functional>

namespace scene {

class Material;
class SubMesh;

struct SubMeshCreateInfo {
    // optional name i guess
    const std::string &name = "";

    // need to know how our vertices are organized into primitives obvs
    VkPrimitiveTopology meshTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // this might work as a default idk

    // i'll try have the meshes supply all material (texture) info up-front
    std::vector<std::shared_ptr<Material>> materials = {};
    std::map<std::string, unsigned int> materialNames = {};

    VertexAttributeFlags enabledVertexAttributes = 0;
    VkDeviceSize vertexCount = 0;
    void *vertexData = nullptr; 

    std::vector<unsigned int> indices = {};

    // optional per-submesh update callback
    std::function<bool(SubMesh &)> update;
};

struct MeshCreateInfo {
    // pointer to parent scene
    std::shared_ptr<scene::Scene> parentScene_;
    
    // Meshes mainly wrap up a set of SubMeshes
    std::vector<SubMeshCreateInfo> submeshCreateInfos = {};

    bool enableSubMeshUpdateCallbacks = true; // by default, it'll look for submesh update callbacks; else it will only 
    // call the whole-mesh callback, in case you want to update the submeshes that way, or if you want to
    // provide a default update callback for submeshes that don't have their own
    std::function<bool(Mesh&)> updateCallback;

};

class SubMesh : public Renderable {
  public:
    SubMesh(const SubMeshCreateInfo &info);
    ~SubMesh();

    bool isSubMesh() override {
        return true;
    }

    // should return true if the geometry has changed, false otherwise
    bool update();

    unsigned int getSizeInBytes() {
        return numVertices_ * vertexSize_ + indices_.size() * sizeof(unsigned int);
    }

    unsigned int getNumVertices() {
        return numVertices_;
    }
    
    unsigned int getVertexStride() {
        return vertexSize_;
    }

    const void *getVertexData() {
        return vertexData_;
    }

    // even though we're using a std::vector for indices, i'll still provide a void* getter
    const void *getIndexData() {
        return indices_.data();
    }

    unsigned int getVertexDataSizeInBytes() {
        return numVertices_ * vertexSize_;
    }

    unsigned int getIndexDataSizeInBytes() {
        return indices_.size() * sizeof(unsigned int);
    }

  private:
    // optional submesh name
    std::string name_;

    VertexAttributeFlags enabledVertexAttributes_;
    unsigned int vertexSize_ = 0;
    unsigned int numVertices_ = 0;
    void *vertexData_ = nullptr;

    std::vector<unsigned int> indices_ = {};

    std::vector<std::shared_ptr<Material>> materials_ = {};
    std::map<std::string, unsigned int> materialNames_ = {}; 
    std::map<UniqueID::uid_t, unsigned int> materialIDs_ = {};

    // optional submesh-specific update callback
    std::function<bool(SubMesh &)> update_;
};

class Mesh : public Renderable {
  public:
    Mesh(const MeshCreateInfo &info);
    ~Mesh();

    bool isMesh() override;

    bool update();

    const std::string &getName() {
        return name_;
    }

    const std::vector<std::shared_ptr<SubMesh>> getSubMeshes() {
        return submeshes_;
    }    

  private:
    /* MESH STATE */
    std::string name_; // now that renderables have a uid, might make sense to refactor around that and excise the string name
    // from objects that will make use of the uid stuff

    bool enableSubMeshUpdateCallbacksOnly_ = true;
    std::vector<std::shared_ptr<SubMesh>> submeshes_ = {};

    unsigned long long meshSizeInBytes_ = 0;

    /* MESH CALLBACKS */
    // update: by convention i think this will just involve making per-frame changes to the mesh
    // if there are no changes to be made, this can return false every time, and no data will be copied
        // therefore to implement a static mesh, we just provide no update callback... hopefully that works?
    std::function<bool(Mesh&)> updateCallback_;

};

}