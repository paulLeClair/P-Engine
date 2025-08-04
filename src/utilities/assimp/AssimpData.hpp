//
// Created by paull on 2025-06-20.
//
#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "../UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::util::assimp {
    class SkeletalAnimation;

    struct Material {
        // TODO
    };

    struct Bone {
        std::string name;
        UniqueIdentifier uid;
        glm::mat4 offset;

        std::vector<std::pair<uint32_t, float> > vertexWeights;
    };

    struct SkeletonNode {
        std::string name;
        int32_t parentIndex;
        glm::mat4 localTransform;

        std::optional<Bone> bone;
    };

    /*
     * This is the static mesh data that we'll read directly from assimp; it will naturally be expanded as we go
     */
    struct Mesh {
        // metadata
        std::string name;
        UniqueIdentifier uid;
        uint32_t vertexCount;
        uint32_t indexCount;

        // vertex attribs; TODO -> make this more general-purpose
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<std::vector<uint32_t> > animationBoneIndices;
        std::vector<std::vector<float> > animationWeights;

        // primitives (just indices for now)
        std::vector<uint32_t> indices;

        std::vector<Bone> bones;
    };
}
