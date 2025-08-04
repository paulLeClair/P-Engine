//
// Created by paull on 2024-07-21.
//

#pragma once

#include <string>

#include "../../../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../Bone/Bone.hpp"

namespace pEngine::girEngine::scene::model::anim {
    /**
     * Since assimp animates things at a node level, and nodes can contain multiple meshes
     * which themselves can contain multiple bones, it makes sense (I hope) to be grouping up
     * connected bones into a group which are all being affected by the same aiNodeAnim from assimp.
     *
     *
     */
    class SkeletonNode {
    public:
        struct CreationInput {
            /**
             * This should correspond to the assimp node that's being animated, if you're using assimp I guess
             */
            std::string nodeName;

            util::UniqueIdentifier uniqueIdentifier;

            std::vector<std::shared_ptr<Bone> > bones;

            std::vector<std::shared_ptr<SkeletonNode> > children;

            /**
             * This is the LOCAL node transform
             */
            glm::mat4 localTransform;
        };

        explicit SkeletonNode(const CreationInput &creationInput) : nodeName(creationInput.nodeName),
                                                                    uniqueIdentifier(creationInput.uniqueIdentifier),
                                                                    bones(creationInput.bones),
                                                                    children(creationInput.children),
                                                                    localTransform(creationInput.localTransform) {
        }

        const std::string nodeName;
        const UniqueIdentifier uniqueIdentifier;

        const std::vector<std::shared_ptr<anim::Bone> > bones;

        const std::vector<std::shared_ptr<SkeletonNode> > children;

        // we'll store the node-level transform in the skeleton node to handle cases where the node has no bones but
        // has transforms that apply to its children
        const glm::mat4 localTransform;
    };
}
