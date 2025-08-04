//
// Created by paull on 2024-07-14.
//

#pragma once


#include <iostream>
#include <memory>
#include <ranges>

#ifdef WIN32
#include <Windows.h>
#endif

#include <complex>
#include <vector>

#include "Skeleton/Skeleton.hpp"
#include "Keyframe/Keyframe.hpp"

#include <glm/gtc/quaternion.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>

// no idea what to use for this, but since we're hopefully gonna be interpolating I think this should be okay
#define DEFAULT_NUMBER_OF_KEYFRAMES 120

// another ugly temporary macro
#define MAX_BONES 255

namespace pEngine::girEngine::scene::model::anim {
    /**
     * This should just kinda aggregate all the data needed to provide animation data to a vertex shader.
     *
     * Fundamentally it will be based around tracking a skeleton of bones and storing keyframes
     * which map to a transformation for each bone in the skeleton and act as a particular pose.
     * In the future it could probably be pretty easy to add interpolation of poses
     * and also expand this to be less rigid in what kinds of animation it allows you to do.
     *
     * For now this can only be built through Assimp, but once that works I'll write my own interface
     * informed by however that works.
     */
    class Animation {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uniqueIdentifier;

            const aiAnimation *animation;

            std::shared_ptr<Skeleton> skeleton;
            // I'm actually fairly sure we want to build this when reading from assimp; in that case, we ignore this

            boost::optional<int> numberOfKeyframesToGenerate = boost::none;

            uint32_t bonePosesUniformBufferBindingIndex = 0;
            glm::mat4 globalInverseTransform;

            std::vector<Keyframe> keyframes = {};

            double animationTicksPerSecond = 0;
            double animationDurationInSeconds = 0;
        };


        explicit Animation(const CreationInput &creationInput) : name(creationInput.name),
                                                                 uniqueIdentifier(creationInput.uniqueIdentifier),
                                                                 skeleton(creationInput.skeleton),
                                                                 bonePosesBindingIndex(
                                                                     creationInput.bonePosesUniformBufferBindingIndex),
                                                                 animationTicksPerSecond(
                                                                     creationInput.animation->mTicksPerSecond),
                                                                 animationDurationInSeconds(
                                                                     creationInput.animation->mDuration / creationInput.
                                                                     animation->mTicksPerSecond) {
            if (!creationInput.keyframes.empty()) {
                keyframes = creationInput.keyframes;
            }

            // disable old code for now
            if (false) {
                oldConstructorCode(creationInput);
            }
        }

        [[nodiscard]] const std::shared_ptr<Skeleton> &getSkeleton() const {
            return skeleton;
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const UniqueIdentifier &getUniqueIdentifier() const {
            return uniqueIdentifier;
        }

        [[nodiscard]] const std::vector<Keyframe> &getKeyframes() const {
            return keyframes;
        }

        [[nodiscard]] double getAnimationDurationInSeconds() const {
            return animationDurationInSeconds;
        }

        [[nodiscard]] double getAnimationTicksPerSecond() const {
            return animationTicksPerSecond;
        }


        /**
         * For now, this will correspond to the descriptor set binding index for the animation bone poses
         * data
         */
        [[nodiscard]] uint32_t getBonePosesBindingIndex() const {
            return bonePosesBindingIndex;
        }

        /**
         * This doesn't seem to work properly! Disabling for now
         * @param currentTimePointInSeconds
         * @param animationDurationInSeconds
         * @param animationTicksPerSecond
         * @param keyframes
         * @return
         */
        static std::vector<glm::mat4> obtainBonePoses(double currentTimePointInSeconds,
                                                      const double animationDurationInSeconds,
                                                      const double animationTicksPerSecond,
                                                      // keyframe index is given as a reference because this function needs to be updating the value as a side effect (refactor later)
                                                      const std::vector<anim::Keyframe> &keyframes
        ) {
            // FOR NOW -> animations are hardcoded to loop endlessly

            // initialize return vector
            // NOTE -> this is using bone groups now, so we can't just 1-1 with the returned bone poses;
            // some sort of unrolling will have to happen here (as is mentioned in another comment)
            std::vector<glm::mat4> interpolatedBonePoses = {};
            interpolatedBonePoses.assign(MAX_BONES, glm::mat4(1.0));

            const double t = fmod(currentTimePointInSeconds, animationDurationInSeconds) * animationTicksPerSecond;


            // DEBUG: this seems to be what's causing the weird pose error :)
            // find the keyframes that form the lower and upper bounds of our current time t
            int searchIndex = 0;
            for (; searchIndex < keyframes.size() - 1; searchIndex++) {
                if (t < keyframes[searchIndex + 1].timePointInTicks) {
                    break;
                }
            }
            if (searchIndex == keyframes.size() - 1) {
                return keyframes[searchIndex].finalBoneTransforms;
            }
            // DEBUG/TEMP:
            // return keyframes[searchIndex].finalBoneTransforms;

            const auto &startKeyframe = keyframes[searchIndex];
            const auto &endKeyframe = keyframes[searchIndex + 1];

            // debug:
            // const auto &startKeyframe = keyframes[47];
            // const auto &endKeyframe = keyframes[48];

            // interpolate between the two keyframes for each of the bone poses, store them in a vector
            const std::vector<glm::mat4> &startTransforms = startKeyframe.finalBoneTransforms;
            const std::vector<glm::mat4> &endTransforms = endKeyframe.finalBoneTransforms;

            if (startTransforms.size() != endTransforms.size()) {
                // TODO - error handling / logging! no throwing!
                throw std::runtime_error(
                    "Error in Animation::obtainBosePoses() -> keyframe transform arrays don't match!");
            }

            // this gives a parameterization of where we are between the two keyframes
            const double interpolationParameter
                    = (t - static_cast<double>(startKeyframe.timePointInTicks)) /
                      static_cast<double>(endKeyframe.timePointInTicks - startKeyframe.timePointInTicks);

            for (int boneIndex = 0; boneIndex < startTransforms.size(); boneIndex++) {
                // TODO - evaluate whether this is another spot where multithreading might be helpful...
                // interpolate between the matrices and store it in the vector we're gonna return
                // (using glm's wonderful mix() function to perform the interpolation)
                // NOTE -> we might need to verify that this method of interpolation is going to work;
                // worst case we may have to keep all the different sub-transforms separate and then combine
                // only at the end so that we can properly interp rotations. hopefully not an issue tho
                interpolatedBonePoses[boneIndex]
                        = mix(
                            startTransforms[boneIndex],
                            endTransforms[boneIndex],
                            interpolationParameter);

                // TODO -> validation!
            }

            // finally we would just return the interpolated bone poses
            return interpolatedBonePoses;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        std::shared_ptr<Skeleton> skeleton;

        std::vector<Keyframe> keyframes;
        int lastKeyframeIndex = 0;

        double animationDurationInSeconds;
        double animationTicksPerSecond;

        uint32_t bonePosesBindingIndex = 0;

        // EVERYTHING (?) PAST HERE IS LIKELY DEPRECATED
        void oldConstructorCode(const CreationInput &creationInput) {
            // first obtain the animation duration in seconds
            animationDurationInSeconds = creationInput.animation->mDuration / creationInput.animation->mTicksPerSecond;

            int numberOfKeyframesToGenerate = creationInput.numberOfKeyframesToGenerate.get_value_or(
                DEFAULT_NUMBER_OF_KEYFRAMES);


            long long currentKeyframeTimePointTicks = 0;
            const long long keyframeTimeIntervalTicks = std::chrono::duration<double>(
                                                            animationDurationInSeconds / numberOfKeyframesToGenerate).
                                                        count() * creationInput.animation->mTicksPerSecond;

            // obtain map of node names to animation channel indices
            std::unordered_map<std::string, int> nodeNameToChannelIndex = {};
            for (unsigned int channelIndex = 0; channelIndex < creationInput.animation->mNumChannels; channelIndex++) {
                nodeNameToChannelIndex.insert(
                    {creationInput.animation->mChannels[channelIndex]->mNodeName.C_Str(), channelIndex});
            }

            for (int keyframeIndex = 0; keyframeIndex < numberOfKeyframesToGenerate; keyframeIndex++) {
                // so basically now we generate some number of keyframes!

                // my first naive implementation will attempt to just rip out some keyframes
                // by sampling the animation data at fixed time intervals; if that doesn't work I'll restructure this

                std::vector<glm::mat4> animationPoses = {};
                // animations will be 1-1 with bones, so we'll use this instead
                // NOTE -> this is still bone-centric when we could rewrite it to be node-centric but i'll try it
                animationPoses.assign(skeleton->getNumberOfBones(), glm::mat4(0.0));

                std::map<uint32_t, glm::mat4> nodeTransforms = {};
                unsigned nodeIndex = 0;
                unsigned boneIndex = 0;
                for (auto &linearizedBoneGroup: skeleton->getLinearizedBoneGroups()) {
                    // operating off assumption that this linearization thing is not properly handling parents rn:

                    auto nodeLocalTransform = linearizedBoneGroup.node->localTransform;
                    // if we have an animation channel for this node, obtain the local transform that will be applied
                    // to all bones connected to said channel
                    if (nodeNameToChannelIndex.contains(linearizedBoneGroup.node->nodeName)) {
                        const auto *nodeAnim = creationInput.animation->mChannels[nodeNameToChannelIndex[
                            linearizedBoneGroup.node->nodeName]];

                        glm::vec3 interpolatedPosition = computeInterpolatedPosition(
                            nodeAnim, currentKeyframeTimePointTicks);
                        glm::quat interpolatedRotation = computeInterpolatedRotation(
                            nodeAnim, currentKeyframeTimePointTicks);
                        glm::vec3 interpolatedScaling = computeInterpolatedScaling(
                            nodeAnim, currentKeyframeTimePointTicks);

                        nodeLocalTransform = translate(glm::mat4(1.0f), interpolatedPosition)
                                             * mat4_cast(interpolatedRotation)
                                             * scale(glm::mat4(1.0f), interpolatedScaling);
                    }

                    auto nodeParentIndex = linearizedBoneGroup.parentIndex;
                    auto parentTransforms = glm::mat4(1.0f);
                    if (nodeParentIndex > -1) {
                        parentTransforms = nodeTransforms[nodeParentIndex];
                    }
                    nodeTransforms.insert({
                        nodeIndex, parentTransforms * nodeLocalTransform
                    });

                    for (auto &bone: linearizedBoneGroup.node->bones) {
                        animationPoses[boneIndex] =
                                // TODO -> do we actually need the global inverse thing here? seems to break everything
                                // inverse(creationInput.globalInverseTransform) *
                                parentTransforms *
                                nodeLocalTransform *
                                bone->getInverseBindPose();
                        // * creationInput.globalInverseTransform;

                        boneIndex++;
                    }

                    nodeIndex++;
                }

                keyframes.emplace_back(Keyframe::CreationInput{
                    // name + "Keyframe" + std::to_string(keyframeIndex),
                    // util::UniqueIdentifier(),
                    // currentKeyframeTimePointTicks,
                    // skeleton->getLinearizedBoneGroups(),
                    // {}, // TODO -> remove this
                    // creationInput.globalInverseTransform,
                    // animationPoses
                });

                currentKeyframeTimePointTicks += keyframeTimeIntervalTicks;
            }
        }


        /**
         *
         * @param node_anim
         * @param currentKeyframeTimePointInTicks note that this is in ticks! assimp doesn't directly specify
         * @return
         */
        static glm::vec3 computeInterpolatedPosition(const aiNodeAnim *node_anim,
                                                     const long long currentKeyframeTimePointInTicks) {
            for (uint32_t i = 0; i < node_anim->mNumPositionKeys; i++) {
                double keyTimeInTicks = node_anim->mPositionKeys[i].mTime;
                if (currentKeyframeTimePointInTicks <= keyTimeInTicks) {
                    uint32_t nextIndex = i + 1;
                    const auto currentKey = node_anim->mPositionKeys[i];
                    if (nextIndex < node_anim->mNumPositionKeys) {
                        const auto nextKey = node_anim->mPositionKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;
                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0};
                        }
                        const aiVector3d deltaPosition = {
                            interpolationFactor * (nextKey.mValue.x - currentKey.mValue.x),
                            interpolationFactor * (nextKey.mValue.y - currentKey.mValue.y),
                            interpolationFactor * (nextKey.mValue.z - currentKey.mValue.z)
                        };
                        const aiVector3d result = {
                            currentKey.mValue.x + deltaPosition.x,
                            currentKey.mValue.y + deltaPosition.y,
                            currentKey.mValue.z + deltaPosition.z
                        };
                        return {result.x, result.y, result.z};
                    }
                    return {currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z};
                }
            }
            // LOG!
            return {0, 0, 0};
        }

        static glm::quat computeInterpolatedRotation(const aiNodeAnim *node_anim,
                                                     const long long currentKeyframeTimePointInTicks) {
            for (uint32_t i = 0; i < node_anim->mNumRotationKeys; i++) {
                double keyTimeInTicks = node_anim->mRotationKeys[i].mTime;
                if (currentKeyframeTimePointInTicks <= keyTimeInTicks) {
                    uint32_t nextIndex = i + 1;
                    if (nextIndex < node_anim->mNumRotationKeys) {
                        const auto currentKey = node_anim->mRotationKeys[i];
                        const auto nextKey = node_anim->mRotationKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;
                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0, 0};
                        }
                        const aiQuaternion &currentKeyRotation = currentKey.mValue;
                        const aiQuaternion &nextKeyRotation = nextKey.mValue;
                        aiQuaternion result = {};
                        aiQuaternion::Interpolate(result, currentKeyRotation, nextKeyRotation, interpolationFactor);
                        return normalize(glm::quat{result.w, result.x, result.y, result.z});
                    }
                    //default: return last keyframe if we're at the end
                    return {
                        node_anim->mRotationKeys[i].mValue.w,
                        node_anim->mRotationKeys[i].mValue.x,
                        node_anim->mRotationKeys[i].mValue.y,
                        node_anim->mRotationKeys[i].mValue.z
                    };
                }
            }
            // LOG!
            return {0, 0, 0, 0};
        }

        static glm::vec3 computeInterpolatedScaling(const aiNodeAnim *node_anim,
                                                    const long long currentKeyframeTimePointInTicks) {
            for (uint32_t i = 0; i < node_anim->mNumScalingKeys; i++) {
                double keyTimeInTicks = node_anim->mScalingKeys[i].mTime;
                if (currentKeyframeTimePointInTicks <= keyTimeInTicks) {
                    uint32_t nextIndex = i + 1;
                    const auto currentKey = node_anim->mScalingKeys[i];
                    if (nextIndex < node_anim->mNumScalingKeys) {
                        const auto nextKey = node_anim->mScalingKeys[nextIndex];

                        const double startTimeTicks = currentKey.mTime;
                        const double endTimeTicks = nextKey.mTime;
                        const double deltaTimeTicks = endTimeTicks - startTimeTicks;
                        const double interpolationFactor =
                                (currentKeyframeTimePointInTicks - startTimeTicks) / deltaTimeTicks;
                        if (interpolationFactor < 0 || interpolationFactor > 1) {
                            // LOG!
                            return {0, 0, 0};
                        }
                        const aiVector3d deltaScaling = {
                            interpolationFactor * (nextKey.mValue.x - currentKey.mValue.x),
                            interpolationFactor * (nextKey.mValue.y - currentKey.mValue.y),
                            interpolationFactor * (nextKey.mValue.z - currentKey.mValue.z)
                        };
                        const aiVector3d result = {
                            currentKey.mValue.x + deltaScaling.x,
                            currentKey.mValue.y + deltaScaling.y,
                            currentKey.mValue.z + deltaScaling.z
                        };
                        return {result.x, result.y, result.z};
                    }
                    return {currentKey.mValue.x, currentKey.mValue.y, currentKey.mValue.z};
                }
            }
            // LOG!
            return {0, 0, 0};
        }
    };
}
