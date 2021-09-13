#include "../../../../../include/core/p_render/scene/mesh/Mesh.hpp"

using namespace scene;

/* SUBMESH */
SubMesh::SubMesh(const SubMeshCreateInfo &info) : Renderable() {
    name_ = info.name;

    // set up submesh data (these submesh objects should contain the actual geometry data)
    numVertices_ = info.vertexCount;
    vertexData_ = info.vertexData; // we just copy the pointer; data won't be copied til bake()    

    // compute vertex size based on enabled attributes
    const auto &computeVertexSize = [&](const VertexAttributeFlags &enabledVertexAttributes) {
        unsigned int vertexSize = 0;

        if (info.enabledVertexAttributes & VertexAttributeFlagBits::PositionBit) {
            vertexSize += sizeof(VertexAttributeTypes::PositionType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::NormalBit) {
            vertexSize += sizeof(VertexAttributeTypes::NormalType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::ColorBit) {
            vertexSize += sizeof(VertexAttributeTypes::ColorType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::UVBit) {
            vertexSize += sizeof(VertexAttributeTypes::UVType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::TangentBit) {
            vertexSize += sizeof(VertexAttributeTypes::TangentType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::BitangentBit) {
            vertexSize += sizeof(VertexAttributeTypes::BitangentType);
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::BoneIndexBit) {
            //todo
        }
        if (info.enabledVertexAttributes & VertexAttributeFlagBits::BoneWeightsBit) {
            //todo
        }

        return vertexSize;
    };
    vertexSize_ = computeVertexSize(info.enabledVertexAttributes);

    indices_ = info.indices;

    
}

SubMesh::~SubMesh() {

}

bool SubMesh::update() {
    if (update_) 
        return update_(*this);
    return false; // otherwise return false; no update function registered -> no change automatically
}

/* MESH */
Mesh::Mesh(const MeshCreateInfo &info) : Renderable() {
    // here we prepare the vertex data + index data initially, so that it can be 
    // copied into the scene buffer for rendering
    unsigned long long meshSizeInBytes = 0;
    for (int i = 0; i < info.submeshCreateInfos.size(); i++) {
        const auto &submeshCreateInfo = info.submeshCreateInfos[i];
        submeshes_.push_back(std::make_shared<SubMesh>(submeshCreateInfo));
        
        const auto &submesh = submeshes_.back();
        meshSizeInBytes += submesh->getIndexDataSizeInBytes() + submesh->getVertexDataSizeInBytes();
    }
    meshSizeInBytes_ = meshSizeInBytes;

    // any other mesh preperation!
    enableSubMeshUpdateCallbacksOnly_ = info.enableSubMeshUpdateCallbacks;

}

Mesh::~Mesh() {
    // not sure i need an explicit dtor but w/e
}

bool Mesh::isMesh() {
    // obviously meshes should return true here
    return true;
}

bool Mesh::update() {
    // idea here i think: activate update callback
        // not sure what to do here
    if (!enableSubMeshUpdateCallbacksOnly_) {
        return updateCallback_(*this);
    }
    return false;
}
