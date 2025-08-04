//
// Created by paull on 2024-07-14.
//

#pragma once

#include <string>
#include <chrono>
#include <map>

#include <glm/glm.hpp>

#include "../../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../Skeleton/Skeleton.hpp"

#define MAX_BONES 255

namespace pEngine::girEngine::scene::model::anim {
    /**
     *
     */
    class Keyframe {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;

            double keyframeTimePointInTicks;

            boost::optional<std::vector<glm::mat4> > finalKeyframeTransforms = boost::none;

            /**
             * For assimp-loaded models (or really any model you're reading from an existing file format), the encoded
             * scene may already place some transforms on the model (they come prepackaged as the offset matrix of
             * the root node (and I think it applies to the meshes and the bones but I'm not sure)
             *
             * LIKELY DEPRECATED
             */
            glm::mat4 globalInverseTransform = {};

            // LIKELY DEPRECATED
            const std::vector<Skeleton::LinearizedSkeletonNode> linearizedSkeletonNodes = {};
        };


        explicit Keyframe(const CreationInput &creationInput) : name(creationInput.name),
                                                                uniqueIdentifier(creationInput.uniqueIdentifier),
                                                                timePointInTicks(
                                                                    creationInput.keyframeTimePointInTicks),
                                                                globalInverseMatrix(
                                                                    creationInput.globalInverseTransform) {
            // EXPERIMENT/DEBUG -> disabling all this finalBoneTransform computation stuff and computing final transforms externally

            if (creationInput.finalKeyframeTransforms.has_value()) {
                finalBoneTransforms = *creationInput.finalKeyframeTransforms;
            } else {
                // TODO -> log!
            }
        }

        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        long long timePointInTicks;

        /**
         * This is the final transforms for each bone in the bones list (it maps 1-1), so it will
         * include the assimp model's @globalInverseMatrix and the concatenated animation transforms as well as
         * each particular bone's inverse bind pose matrix to transform from bone space to world space
         *
         * ANOTHER THING OF NOTE: i don't know if directly mixing the matrices is valid (it feels like it should be...?)
         * but I think it's probably a good idea to add a dinky lil interp-bypass where it just uses the closest keyframe
         * transforms directly
         */
        std::vector<glm::mat4> finalBoneTransforms = {};

        /**
         * This is meant to cancel out any encoded transformations from the scene file itself; we want to have full
         * control of the positioning so if a model from a file is already being transformed in some way we need to
         * undo those transformations with an inverse matrix.
         */
        glm::mat4 globalInverseMatrix;
    };
}
