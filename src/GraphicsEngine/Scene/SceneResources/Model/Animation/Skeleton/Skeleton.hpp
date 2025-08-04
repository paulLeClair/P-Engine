//
// Created by paull on 2024-07-13.
//

#pragma once

#include "../../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "Bone/Bone.hpp"
#include "BoneGroup/SkeletonNode.hpp"
#include <assimp/scene.h>


// ugly, horrible: copying this macro from the Keyframe class
#define MAX_BONES 255

namespace pEngine::girEngine::scene::model::anim {
    /**
     *
     */
    class Skeleton {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;

            const std::shared_ptr<SkeletonNode> rootBone;
        };

        struct LinearizedSkeletonNode {
            std::shared_ptr<SkeletonNode> node;
            int parentIndex = -1; // -1 signals no parent (root)
        };

        explicit Skeleton(const CreationInput &creationInput)
            : name(creationInput.name), uniqueIdentifier(creationInput.uniqueIdentifier) {
            if (creationInput.rootBone) {
                linearizeSkeletonTreeStartingFromRoot(creationInput.rootBone);

                // after linearization, we wanna sort it by parent index
                std::ranges::sort(linearizedBoneGroups,
                                  [&](const LinearizedSkeletonNode &segmentBindingOne,
                                      const LinearizedSkeletonNode &segmentBindingTwo) {
                                      return segmentBindingOne.parentIndex < segmentBindingTwo.parentIndex;
                                  });

                numberOfBones = computeNumberOfBones();
            }
        }

        [[nodiscard]] const std::vector<LinearizedSkeletonNode> &getLinearizedBoneGroups() const {
            return linearizedBoneGroups;
        }

        // I think we need to be able to provide a data structure containing all the vertex bone weights
        // and bone indices for each vertex; this can be agnostic as to whether the vertices are even valid
        // but it will just traverse the bone hierarchy and obtain all the vertex weights in turn.
        // This is important for building up our animated mesh vertex data
        struct VertexBoneAttachment {
            std::vector<unsigned> boneIndices;
            std::vector<float> boneWeights;
        };

        /**
         * This is not an efficient function; I can probably cache this info but
         * it should only be called once (when setting up a vertex buffer from Assimp information currently)
         * during bone group initialization
         */
        std::vector<VertexBoneAttachment> getVertexBoneIndicesAndWeights(size_t vertexCount) {
            if (!cachedVertexBoneIndicesAndWeights.empty()) {
                return cachedVertexBoneIndicesAndWeights;
            }

            std::vector<VertexBoneAttachment> vertexBoneIndicesAndWeights(vertexCount);
            size_t boneIndex = 0;
            for (auto &boneGroup: linearizedBoneGroups) {
                for (auto &bone: boneGroup.node->bones) {
                    for (auto &vertexWeight: bone->getVertexWeights()) {
                        // make sure we don't exceed our current hard-coded limit of 4 bones per vertex
                        if (vertexBoneIndicesAndWeights[vertexWeight.affectedVertex].boneIndices.size() > 4) {
                            // TODO - logging / actual error handling! avoid throwing!
                            throw std::runtime_error(
                                "Error in Skeleton::getVertexBoneIndicesAndWeights() - too many bones attached to vertex "
                                +
                                std::to_string(vertexWeight.affectedVertex));
                        }

                        vertexBoneIndicesAndWeights[vertexWeight.affectedVertex].boneIndices.push_back(boneIndex);
                        vertexBoneIndicesAndWeights[vertexWeight.affectedVertex].boneWeights.push_back(
                            vertexWeight.weight);
                    }
                    boneIndex++;
                }
            }
            cachedVertexBoneIndicesAndWeights = vertexBoneIndicesAndWeights;
            return vertexBoneIndicesAndWeights;
        }

        [[nodiscard]] int getNumberOfBones() const {
            return numberOfBones;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        // the new thing the skeleton will focus on will be the BoneGroup abstraction;
        // this maps to a single assimp node unlike bones themselves (should still mostly work the same though)
        int numberOfBones = 0;

        std::vector<LinearizedSkeletonNode> linearizedBoneGroups = {};

        std::vector<VertexBoneAttachment> cachedVertexBoneIndicesAndWeights;

        [[nodiscard]] int computeNumberOfBones() {
            int size = 0;

            for (auto &boneGroup: linearizedBoneGroups) {
                size += boneGroup.node->bones.size();
            }

            return size;
        }

        void
        linearizeSkeletonTreeStartingFromRoot(const std::shared_ptr<SkeletonNode> &rootBoneGroup) {
            int nodeIndex = 0;
            int parentNodeIndex = -1;
            linearizedBoneGroups = {};
            // okay so here we need to reconcile the whole bonegroup thing
            linearizeRecursive(linearizedBoneGroups, rootBoneGroup, nodeIndex, parentNodeIndex);
        }

        void
        linearizeRecursive(std::vector<LinearizedSkeletonNode> &linearizedSegments,
                           const std::shared_ptr<SkeletonNode> &currentBoneGroup,
                           int &nodeIndex, int parentNodeIndex) {
            // place current node in list
            linearizedSegments.push_back(
                {
                    currentBoneGroup,
                    parentNodeIndex
                });

            // place all child nodes into list
            int currentNodeIdentifier = nodeIndex;
            for (const std::shared_ptr<SkeletonNode> &childBoneGroup: currentBoneGroup->children) {
                nodeIndex++;
                linearizeRecursive(linearizedSegments, childBoneGroup, nodeIndex, currentNodeIdentifier);
            }
        }
    };
}
