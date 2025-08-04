//
// Created by paull on 2025-06-18.
//

#include "SkeletalAnimation.hpp"

#include <ranges>

#include "SceneImporter.hpp"

namespace pEngine::util::assimp {
    std::vector<glm::mat4> SkeletalAnimation::obtainAnimationPoses(const double timePointInSeconds,
                                                                   std::unordered_map<std::string,
                                                                       AnimationChannel> &animationChannels) {
        if (!skeletonRootNode || !animation) {
            // TODO -> log!
            return {};
        }

        if (linearizedSkeletonNodes.empty() || linearizedBones.empty() || boneNamesToIndices.empty()) {
            // TODO -> log!
            return {};
        }

        const uint32_t boneCount = linearizedBones.size();

        std::vector<glm::mat4> finalBonePoses;
        finalBonePoses.assign(boneCount, glm::mat4(1.0f));
        std::vector<glm::mat4> localBonePoses;
        localBonePoses.assign(boneCount, glm::mat4(1.0f));

        const double currentTimePointInTicks = fmod(timePointInSeconds * animation->mTicksPerSecond,
                                                    animation->mDuration);
        cache.timeInTicks = currentTimePointInTicks;

        if (cacheDebugInfo) {
            cache.previousKeyframe.keyframePositions.resize(linearizedBones.size());
            cache.nextKeyframe.keyframePositions.resize(linearizedBones.size());

            cache.previousKeyframe.keyframeRotations.resize(linearizedBones.size());
            cache.nextKeyframe.keyframeRotations.resize(linearizedBones.size());

            cache.previousKeyframe.keyframeScales.resize(linearizedBones.size());
            cache.nextKeyframe.keyframeScales.resize(linearizedBones.size());

            cache.previousKeyframe.finalizedBonePoses.assign(linearizedBones.size(), glm::mat4(1.0f));
            cache.nextKeyframe.finalizedBonePoses.assign(linearizedBones.size(), glm::mat4(1.0f));
        }

        auto traverseSkeletonAndConstructFinalBonePoses = [&](
            const aiNode *currentNode,
            const glm::mat4 &parentTransforms,
            const glm::mat4 &prevParent = glm::mat4(1.0f),
            const glm::mat4 &nextParent = glm::mat4(1.0f)) -> void {
            /**
             * Recursive lambda which takes a self-reference
             */
            auto traverseSkeletonAndConstructFinalBonePosesImpl = [&](
                const aiNode *skeletonNode,
                const glm::mat4 &parentNodeTransform,
                const glm::mat4 &previousKeyframeParentTransforms = glm::mat4(1.0f),
                const glm::mat4 &nextKeyframeParentTransforms = glm::mat4(1.0f),
                auto &recursiveFunctionRef = [&] {
                }) mutable -> void {
                if (!skeletonNode) {
                    // TODO -> log!
                    return;
                }

                // first, see whether our current node is animated by iterating over all channels
                const std::string currentNodeName = skeletonNode->mName.C_Str();
                // const aiNodeAnim *nodeAnim = nullptr;
                AnimationChannel *nodeAnim = nullptr;
                bool found = false;
                for (const auto &nodeName: animationChannels | std::views::keys) {
                    if (nodeName == currentNodeName) {
                        found = true;
                        nodeAnim = &animationChannels.at(nodeName);
                        break;
                    }
                }
                if (!found) {
                    // unset the node anim pointer, since we don't want to just use the last node if we don't find it
                    nodeAnim = nullptr;
                }

                glm::mat4 localTransform = convert_ai_matrix_to_glm(skeletonNode->mTransformation);

                // if we have an animation for this node, we recompute the local transform instead of using
                // the static assimp transform that was baked into the node
                if (nodeAnim) {
                    const glm::vec3 interpolatedPosition =
                            computeInterpolatedPosition(
                                *nodeAnim, currentTimePointInTicks);
                    const glm::quat interpolatedRotation =
                            computeInterpolatedRotation(
                                *nodeAnim, currentTimePointInTicks);

                    glm::vec3 interpolatedScaling
                            = computeInterpolatedScaling(
                                *nodeAnim, currentTimePointInTicks);

                    if (interpolatedScaling == glm::vec3(0.0f)) {
                        interpolatedScaling = glm::vec3(1.0f);
                    }

                    localTransform = translate(glm::mat4(1.0f), interpolatedPosition)
                                     * mat4_cast(interpolatedRotation)
                                     * scale(glm::mat4(1.0f), interpolatedScaling);
                }
                auto debugCachePrevKeyParentTransform = previousKeyframeParentTransforms * convert_ai_matrix_to_glm(
                                                            skeletonNode->mTransformation);
                auto debugCacheNextKeyParentTransform = nextKeyframeParentTransforms * convert_ai_matrix_to_glm(
                                                            skeletonNode->mTransformation);
                if (const auto boneIndexIt = boneNamesToIndices.find(currentNodeName);
                    boneIndexIt != boneNamesToIndices.end()) {
                    const auto boneIndex = boneIndexIt->second;
                    const Bone &bone = linearizedBones[boneIndex];

                    finalBonePoses[boneIndex] =
                            inverseRootTransform
                            * parentNodeTransform
                            * localTransform
                            * bone.offset;


                    if (cacheDebugInfo && nodeAnim) {
                        // we'll also compute final bone poses for the other keyframes I guess...
                        auto channel = animationChannels[nodeAnim->nodeName];

                        /* COMPUTE FINAL BONE POSES FOR PREVIOUS KEYFRAME */
                        uint32_t previousKeyframePositionIndex = cache.previousKeyframe.keyframePositions[boneIndex];
                        uint32_t previousKeyframeRotationIndex = cache.previousKeyframe.keyframeRotations[boneIndex];
                        uint32_t previousKeyframeScaleIndex = cache.previousKeyframe.keyframeScales[boneIndex];

                        glm::mat4 previousFrameLocalPositionTransform = translate(
                            glm::mat4(1.0f), channel.positionKeys[previousKeyframePositionIndex].position);
                        glm::mat4 previousFrameLocalRotationTransform = glm::mat4_cast(
                            channel.rotationKeys[previousKeyframeRotationIndex].rotation);
                        glm::mat4 previousFrameLocalScaling = glm::scale(
                            glm::mat4(1.0f), channel.scaleKeys[previousKeyframeScaleIndex].scale);
                        auto previousKeyframeLocalTransform = previousFrameLocalPositionTransform
                                                              * previousFrameLocalRotationTransform
                                                              * previousFrameLocalScaling;
                        cache.previousKeyframe.finalizedBonePoses[boneIndex] =
                                inverseRootTransform *
                                previousKeyframeParentTransforms *
                                previousKeyframeLocalTransform * bone.offset;

                        /* COMPUTE FINAL BONE POSES FOR NEXT KEYFRAME */
                        uint32_t nextKeyframePositionIndex = cache.nextKeyframe.keyframePositions[boneIndex];
                        uint32_t nextKeyframeRotationIndex = cache.nextKeyframe.keyframeRotations[boneIndex];
                        uint32_t nextKeyframeScaleIndex = cache.nextKeyframe.keyframeScales[boneIndex];

                        glm::mat4 nextFrameLocalPositionTransform = translate(
                            glm::mat4(1.0f), channel.
                            positionKeys[nextKeyframePositionIndex].position);
                        glm::mat4 nextFrameLocalRotationTransform = glm::mat4_cast(
                            channel.rotationKeys[nextKeyframeRotationIndex].rotation);
                        glm::mat4 nextFrameLocalScaling = glm::scale(
                            glm::mat4(1.0f), channel.scaleKeys[nextKeyframeScaleIndex].scale);
                        auto nextKeyframeLocalTransform = nextFrameLocalPositionTransform
                                                          * nextFrameLocalRotationTransform
                                                          * nextFrameLocalScaling;
                        // NOTE: our poses aren't perfect for the previous/next keyframes; not sure if that's something
                        // that also extends to the final animation poses but it doesn't seem to
                        cache.nextKeyframe.finalizedBonePoses[boneIndex] =
                                inverseRootTransform *
                                nextKeyframeParentTransforms *
                                nextKeyframeLocalTransform * bone.offset;

                        // pass through previous/next keyframe poses if they are active
                        debugCachePrevKeyParentTransform =
                                previousKeyframeParentTransforms * previousKeyframeLocalTransform;
                        debugCacheNextKeyParentTransform = nextKeyframeParentTransforms * nextKeyframeLocalTransform;
                    }
                }

                // finally, recurse over all the children of this node whether it's a bone or non-bone node
                for (uint32_t childIndex = 0; childIndex < skeletonNode->mNumChildren; childIndex++) {
                    recursiveFunctionRef(
                        skeletonNode->mChildren[childIndex],
                        parentNodeTransform * localTransform,
                        debugCachePrevKeyParentTransform,
                        debugCacheNextKeyParentTransform,
                        recursiveFunctionRef
                    );
                }
            };

            // note: to "easily" enable recursive lambdas, we're using the technique where you pass in
            // a reference to itself so that it can resolve the recursive call
            traverseSkeletonAndConstructFinalBonePosesImpl(
                currentNode,
                parentTransforms,
                prevParent,
                nextParent,
                traverseSkeletonAndConstructFinalBonePosesImpl
            );
        };

        // call our outer-level activator function for our recursive lambda
        traverseSkeletonAndConstructFinalBonePoses(
            skeletonRootNode,
            rootTransform,
            rootTransform,
            rootTransform
        );

        // cache previous time
        lastTime = currentTimePointInTicks;

        if (cacheDebugInfo) {
            cache.finalizedBonePoses = finalBonePoses;
        }

        return finalBonePoses;
    }

    bool SkeletalAnimation::recursiveLinearize(const std::vector<Bone> &globalBonesList,
                                               const aiNode *node,
                                               int32_t parentIndex) {
        if (globalBonesList.empty()) {
            // TODO -> log!
            return false;
        }

        const auto bone_itr = std::ranges::find_if(globalBonesList, [&](const auto &bone) {
            return bone.name == node->mName.C_Str();
        });

        auto localTransform = convert_ai_matrix_to_glm(node->mTransformation);
        if (bone_itr != globalBonesList.end()) {
            linearizedSkeletonNodes.emplace_back(
                bone_itr->name,
                parentIndex,
                localTransform,
                std::make_optional<Bone>(*bone_itr)
            );
        } else {
            linearizedSkeletonNodes.emplace_back(
                node->mName.C_Str(),
                parentIndex,
                localTransform,
                std::nullopt
            );
        }
        const uint32_t currentIndex = linearizedSkeletonNodes.size() - 1;
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            if (!recursiveLinearize(globalBonesList, node->mChildren[i], currentIndex)) return false;
        }
        return true;
    }

    float SkeletalAnimation::calculate_interpolation_factor(const double currentKeyframeTimePointInTicks,
                                                            const double startTime, const double endTime) {
        return (currentKeyframeTimePointInTicks - startTime) / (endTime - startTime);
    }

    glm::vec3 SkeletalAnimation::computeInterpolatedPosition(AnimationChannel &channel,
                                                             const double currentKeyframeTimePointInTicks) {
        if (channel.positionKeys.empty()) return {}; //TODO -> log!
        if (channel.positionKeys.size() == 1) return channel.positionKeys.back().position;

        uint32_t frame = 0;
        for (frame = currentKeyframeTimePointInTicks >= lastTime ? channel.lastPositionKey : 0;
             frame < channel.positionKeys.size() - 1; frame++) {
            if (currentKeyframeTimePointInTicks < channel.positionKeys[frame + 1].time) {
                break;
            }
        }
        channel.lastPositionKey = frame;

        const uint32_t nextFrame = (frame + 1) % channel.positionKeys.size();
        const auto &key = channel.positionKeys[frame];
        const auto &nextKey = channel.positionKeys[nextFrame];
        double diffTime = nextKey.time - key.time;
        if (diffTime < 0.0) {
            // wraparound
            diffTime += animation->mDuration;
        }

        if (cacheDebugInfo) {
            cache.timeInTicks = currentKeyframeTimePointInTicks;

            const uint32_t boneIndex = boneNamesToIndices[channel.nodeName];
            cache.previousKeyframe.keyframePositions[boneIndex] = frame;
            cache.nextKeyframe.keyframePositions[boneIndex] = nextFrame;
        }
        if (diffTime > 0) {
            const auto factor = static_cast<float>((currentKeyframeTimePointInTicks - key.time) / diffTime);
            cache.interpolationFactor = factor;
            return glm::mix(key.position, nextKey.position, factor);
        }
        return key.position;
    }

    glm::quat SkeletalAnimation::computeInterpolatedRotation(AnimationChannel &channel,
                                                             const double currentKeyframeTimePointInTicks) {
        if (channel.rotationKeys.empty()) return {}; //TODO -> log!
        if (channel.rotationKeys.size() == 1) return channel.rotationKeys.back().rotation;

        uint32_t frame = 0;
        for (frame = currentKeyframeTimePointInTicks >= lastTime ? channel.lastRotationKey : 0;
             frame < channel.rotationKeys.size() - 1; frame++) {
            if (currentKeyframeTimePointInTicks < channel.rotationKeys[frame + 1].time) {
                break;
            }
        }
        channel.lastRotationKey = frame;

        uint32_t nextFrame = (frame + 1) % channel.rotationKeys.size();
        const auto &[time, rotation] = channel.rotationKeys[frame];
        const auto &[nextKey, nextRotation] = channel.rotationKeys[nextFrame];
        double diffTime = nextKey - time;
        if (diffTime < 0.0) {
            // wraparound
            diffTime += animation->mDuration;
        }

        if (cacheDebugInfo) {
            cache.timeInTicks = currentKeyframeTimePointInTicks;

            const uint32_t boneIndex = boneNamesToIndices[channel.nodeName];
            cache.previousKeyframe.keyframeRotations[boneIndex] = frame;
            cache.nextKeyframe.keyframeRotations[boneIndex] = nextFrame;
        }
        if (diffTime > 0) {
            const auto factor = static_cast<float>((currentKeyframeTimePointInTicks - time) / diffTime);
            return glm::slerp(rotation, nextRotation, factor);
        }
        return rotation;
    }

    glm::vec3 SkeletalAnimation::computeInterpolatedScaling(AnimationChannel &channel,
                                                            const double currentKeyframeTimePointInTicks) {
        if (channel.scaleKeys.empty()) return {}; //TODO -> log!
        if (channel.scaleKeys.size() == 1) return channel.scaleKeys.back().scale;

        uint32_t frame = 0;
        for (frame = currentKeyframeTimePointInTicks >= lastTime ? channel.lastScaleKey : 0;
             frame < channel.scaleKeys.size() - 1; frame++) {
            if (currentKeyframeTimePointInTicks < channel.scaleKeys[frame + 1].time) {
                break;
            }
        }
        channel.lastScaleKey = frame;

        const uint32_t nextFrame = (frame + 1) % channel.scaleKeys.size();
        const auto &key = channel.scaleKeys[frame];
        const auto &nextKey = channel.scaleKeys[nextFrame];
        double diffTime = nextKey.time - key.time;
        if (diffTime < 0.0) {
            // wraparound
            diffTime += animation->mDuration;
        }

        if (cacheDebugInfo) {
            cache.timeInTicks = currentKeyframeTimePointInTicks;

            const uint32_t boneIndex = boneNamesToIndices[channel.nodeName];
            cache.previousKeyframe.keyframeScales[boneIndex] = frame;
            cache.nextKeyframe.keyframeScales[boneIndex] = nextFrame;
        }
        if (diffTime > 0) {
            const float factor = static_cast<float>((currentKeyframeTimePointInTicks - key.time) / diffTime);
            return glm::mix(key.scale, nextKey.scale, factor);
        }
        return key.scale;
    }
}
