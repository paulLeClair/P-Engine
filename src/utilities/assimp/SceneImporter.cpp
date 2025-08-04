//
// Created by paull on 2025-06-12.
//

#include "SceneImporter.hpp"

#include <set>

#include "assimp/Importer.hpp"

#define MAX_BONES_PER_VERTEX 4

namespace pEngine::util::assimp {
    void SceneImporter::extractMeshData(const aiScene *assimpScene, SkeletalAnimation &modelAnim,
                                        std::vector<Mesh> &meshes, aiNode *meshNode) const {
        for (uint32_t meshIndex = 0; meshIndex < meshNode->mNumMeshes; meshIndex++) {
            auto *mesh = assimpScene->mMeshes[meshNode->mMeshes[meshIndex]];

            // TODO -> add config for which attributes you want
            std::vector<glm::vec3> positions;
            for (uint32_t posIndex = 0; posIndex < mesh->mNumVertices; posIndex++) {
                auto pos = mesh->mVertices[posIndex];
                positions.emplace_back(
                    pos.x, pos.y, pos.z
                );
            }

            std::vector<glm::vec3> normals;
            for (uint32_t normIndex = 0; normIndex < mesh->mNumVertices; normIndex++) {
                auto normal = mesh->mNormals[normIndex];
                normals.emplace_back(normal.x, normal.y, normal.z);
            }

            std::vector<glm::vec2> uvs;
            // Textures are TODO !

            std::vector<Bone> bones = {};
            std::vector<std::vector<uint32_t> > animationBoneIndices;
            animationBoneIndices.assign(mesh->mNumVertices, {});
            std::vector<std::vector<float> > animationWeights;
            animationWeights.assign(mesh->mNumVertices, {});
            if (anim) {
                for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
                    auto bone = mesh->mBones[boneIndex];

                    std::string boneName = bone->mName.C_Str();
                    if (!modelAnim.getBoneNamesToIndicesMap().contains(boneName)) {
                        // TODO -> log a warning/status update
                        continue;
                    }
                    //NEW: map our bone index into the skeletal animation's linearized global bone array
                    uint32_t mappedBoneIndex = modelAnim.getBoneNamesToIndicesMap().at(boneName);
                    std::vector<std::pair<uint32_t, float> > boneVertexWeights;

                    for (uint32_t i = 0; i < bone->mNumWeights; i++) {
                        auto weight = bone->mWeights[i];
                        boneVertexWeights.emplace_back(weight.mVertexId, weight.mWeight);

                        auto &vertBoneIndices = animationBoneIndices[weight.mVertexId];
                        auto &vertBoneWeights = animationWeights[weight.mVertexId];
                        if (vertBoneIndices.size() > MAX_BONES_PER_VERTEX
                            || vertBoneWeights.size() > MAX_BONES_PER_VERTEX)
                            continue;
                        vertBoneIndices.emplace_back(mappedBoneIndex);
                        vertBoneWeights.emplace_back(weight.mWeight);
                    }
                    auto &offsetMatrix = bone->mOffsetMatrix;
                    bones.push_back({
                        .name = boneName,
                        .uid = UniqueIdentifier(),
                        .offset = SkeletalAnimation::convert_ai_matrix_to_glm(offsetMatrix),
                        .vertexWeights = boneVertexWeights,
                    });
                }
            }

            // primitives (just indices for now)
            // NOTE -> since we don't know if we actually have 3 verts per face (may be lines in there too)
            // we'll just obtain it by iterating over our face array
            uint32_t indexCount = 0;
            std::vector<uint32_t> indices = {};
            for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
                auto face = mesh->mFaces[faceIndex];
                indexCount += face.mNumIndices;
                for (uint32_t i = 0; i < face.mNumIndices; i++) {
                    indices.emplace_back(face.mIndices[i]);
                }
            }

            meshes.push_back({
                .name = std::string(mesh->mName.C_Str()),
                .uid = UniqueIdentifier(),
                .vertexCount = mesh->mNumVertices,
                .indexCount = indexCount,
                .positions = positions,
                .normals = normals,
                .uvs = uvs,
                .animationBoneIndices = animationBoneIndices,
                .animationWeights = animationWeights,
                .indices = indices,
                .bones = bones,
            });
        }
    }

    bool SceneImporter::import(const aiScene *assimpScene) {
        if (!assimpScene) {
            // TODO -> log!
            return false;
        }

        const aiNode *rootNode = assimpScene->mRootNode;

        if (rootNode->mNumChildren == 0) {
            return false;
        }
        aiNode *skeletonRootNode = rootNode->mChildren[0];

        // ERROR: this does not work as intended. we end up with 2 skeleton nodes here and choose the wrong one
        std::vector<aiNode *> skeletonNodes = {};

        bool hasSingleChildWithMeshes = rootNode->mNumChildren == 1 && !nodeTreeContainsNoMeshes(skeletonRootNode);
        if (hasSingleChildWithMeshes) {
            // in this case, we want to iterate over the children of the child instead;
            auto *rootChild = rootNode->mChildren[0];
            for (uint32_t childOfChildIndex = 0; childOfChildIndex < rootChild->mNumChildren; childOfChildIndex++) {
                if (auto child = rootChild->mChildren[childOfChildIndex];
                    nodeTreeContainsNoMeshes(child)) {
                    skeletonNodes.push_back(child);
                    break; // NEW: ensure we only use the first one (still might not be right tho)
                }
            }
        } else {
            for (uint32_t rootChildIndex = 0; rootChildIndex < rootNode->mNumChildren; rootChildIndex++) {
                if (auto child = rootNode->mChildren[rootChildIndex];
                    nodeTreeContainsNoMeshes(child)) {
                    skeletonNodes.push_back(child);
                    break; // NEW: ensure we only use the first one (still might not be right tho)
                }
            }
        }

        if (skeletonNodes.empty()) {
            return false;
        }
        skeletonRootNode = skeletonNodes.back();

        if (hasSingleChildWithMeshes) {
            // in this case, we assume our root's child node is effectively the root of our scene
            rootTransform = SkeletalAnimation::convert_ai_matrix_to_glm(rootNode->mChildren[0]->mTransformation);
        } else {
            rootTransform = SkeletalAnimation::convert_ai_matrix_to_glm(rootNode->mTransformation);
        }

        aiAnimation *assimpAnim = nullptr;
        if (assimpScene->mNumAnimations != 1) {
            // TODO -> log!
            return false;
        }
        assimpAnim = assimpScene->mAnimations[0];
        anim = assimpAnim;
        // TODO -> support non-animated meshes here lol

        SkeletalAnimation modelAnim = {};
        if (anim) {
            // 1. build the skeletal animation first; this way we can have a linearized bones list
            // 1.a -> build preliminary list of bones that will be linearized into skeleton traversal order
            std::vector<Bone> unprocessedGlobalBonesList = {};
            std::set<std::string> encounteredBoneNames = {};
            for (uint32_t m = 0; m < assimpScene->mNumMeshes; m++) {
                for (uint32_t b = 0; b < assimpScene->mMeshes[m]->mNumBones; b++) {
                    auto bone = assimpScene->mMeshes[m]->mBones[b];

                    if (std::string boneName = bone->mName.C_Str(); !encounteredBoneNames.contains(boneName)) {
                        std::vector<std::pair<uint32_t, float> > weights = {};
                        for (uint32_t w = 0; w < bone->mNumWeights; w++) {
                            auto weight = bone->mWeights[w];
                            weights.emplace_back(weight.mVertexId, weight.mWeight);
                        }
                        unprocessedGlobalBonesList.emplace_back(
                            boneName,
                            UniqueIdentifier(),
                            SkeletalAnimation::convert_ai_matrix_to_glm(bone->mOffsetMatrix),
                            weights
                        );
                        encounteredBoneNames.insert(boneName);
                    }
                }
            }
            // 1.b -> construct the skeletal animation
            modelAnim = SkeletalAnimation(
                skeletonRootNode,
                anim,
                unprocessedGlobalBonesList,
                rootTransform
            );
        }

        // 2. use bone names from our skeletal animation to construct our indices (in our mesh construction loop);
        // traverse root node, obtain any meshes
        std::vector<Mesh> meshes = {};
        if (rootNode->mNumChildren == 1 && rootNode->mChildren[0]->mNumChildren > 0) {
            for (uint32_t childOfChildIndex = 0; childOfChildIndex < rootNode->mChildren[0]->mNumChildren;
                 childOfChildIndex++) {
                aiNode *node = rootNode->mChildren[0]->mChildren[childOfChildIndex];
                if (!nodeTreeContainsNoMeshes(node)) {
                    extractMeshData(assimpScene, modelAnim, meshes, node);
                }
            }
        } else {
            for (uint32_t rootChildIndex = 0; rootChildIndex < rootNode->mNumChildren; rootChildIndex++) {
                if (!nodeTreeContainsNoMeshes(rootNode->mChildren[rootChildIndex])) {
                    extractMeshData(assimpScene, modelAnim, meshes, rootNode->mChildren[rootChildIndex]);
                }
            }
        }

        // FOR NOW-> hardcoding this to only work with single-model scenes.
        animatedModels.push_back(AnimatedModel{
            .name = std::string(rootNode->mName.C_Str()),
            .uid = UniqueIdentifier(),
            .materials = {}, // TODO
            .meshes = meshes,
            .animation = modelAnim,
        });
        return true;
    }
}
