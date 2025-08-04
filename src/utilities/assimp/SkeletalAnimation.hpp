//
// Created by paull on 2025-06-18.
//

#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <optional>
#include <set>
#include <assimp/anim.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AssimpData.hpp"
#include "../../lib/murmur3/MurmurHash3.hpp"

namespace pEngine::util::assimp {
    /**
     * This stores a capture of a particular keyframe; when animation debug info caching is enabled,
     * every time a new bone pose is generated it will store relevant information so that afterwards you can
     * see what you're working with.
     */
    struct KeyFrameInfo {
        std::vector<glm::mat4> finalizedBonePoses = {};
        std::vector<uint32_t> keyframePositions = {};
        std::vector<uint32_t> keyframeRotations = {};
        std::vector<uint32_t> keyframeScales = {};
        glm::mat4 accumulatedParentTransforms;
    };

    /**
     * TODO -> move all these info cache structs out of the assimp utility thing and put them in the engine mode
     */
    struct AnimationInfoCache {
        double timeInTicks = 0.0;
        KeyFrameInfo previousKeyframe;
        KeyFrameInfo nextKeyframe;

        std::vector<glm::mat4> finalizedBonePoses;
        float interpolationFactor;
    };

    // thanks to VladH for this idea!
    struct AnimationChannel {
        std::string nodeName; // this is the assimp node name

        struct PositionKey {
            double time;
            glm::vec3 position;
        };

        struct RotationKey {
            double time;
            glm::quat rotation;
        };

        struct ScaleKey {
            double time;
            glm::vec3 scale;
        };

        // sorted maps of keys for this particular node animation
        // plus cached indices for each field
        std::vector<PositionKey> positionKeys;
        uint32_t lastPositionKey = 0;
        std::vector<RotationKey> rotationKeys;
        uint32_t lastRotationKey = 0;
        std::vector<ScaleKey> scaleKeys;
        uint32_t lastScaleKey = 0;
    };

    class SkeletalAnimation {
    public:
        // assimp data structures that we'll read animation poses from
        aiNode *skeletonRootNode = nullptr;
        aiAnimation *animation = nullptr;
        // std::vector<Bone> globalBonesList;

        SkeletalAnimation() = default;


        SkeletalAnimation(aiNode *skeleton_node,
                          aiAnimation *animation,
                          const std::vector<Bone> &globalBonesList,
                          const glm::mat4 &rootTransform)
            : skeletonRootNode(skeleton_node),
              animation(animation),
              rootTransform(rootTransform),
              inverseRootTransform(glm::inverse(rootTransform)) {
            linearizeNodeHierarchy(globalBonesList);
            // copy over *only* the bones from the linearized hierarchy; store {bone name -> index} for each one
            linearizedBones = {};
            boneNamesToIndices.clear();
            for (auto &node: linearizedSkeletonNodes) {
                auto itr = std::ranges::find_if(globalBonesList, [&](const Bone &bone) {
                    return bone.name == node.name;
                });
                if (itr != globalBonesList.end()) {
                    linearizedBones.push_back(*itr);
                    boneNamesToIndices[linearizedBones.back().name] = linearizedBones.size() - 1;
                }
            }

            // init animation channels
            for (uint32_t i = 0; i < animation->mNumChannels; i++) {
                auto &anim_channel = animation->mChannels[i];

                std::vector<AnimationChannel::PositionKey> positionKeys;
                std::vector<AnimationChannel::RotationKey> rotationKeys;
                std::vector<AnimationChannel::ScaleKey> scaleKeys;

                if (anim_channel->mNumPositionKeys == 0) {
                    // TODO -> log!
                    std::cout << "No position keys provided by assimp for channel: " << anim_channel->mNodeName.C_Str()
                            << std::endl;
                }
                if (anim_channel->mNumRotationKeys == 0) {
                    // TODO -> log!
                    std::cout << "No rotation keys  provided by assimp for channel: " << anim_channel->mNodeName.C_Str()
                            << std::endl;
                }
                if (anim_channel->mNumScalingKeys == 0) {
                    // TODO -> log!
                    std::cout << "No scale keys provided by assimp for channel: " << anim_channel->mNodeName.C_Str() <<
                            std::endl;
                }

                for (uint32_t posKey = 0; posKey < anim_channel->mNumPositionKeys; posKey++) {
                    auto &key = anim_channel->mPositionKeys[posKey];
                    positionKeys.emplace_back(key.mTime, glm::vec3(
                                                  key.mValue.x,
                                                  key.mValue.y,
                                                  key.mValue.z
                                              ));
                }

                // due to a strange bug with a previous assimp release (that seems to have been fixed)
                // we have this function for detecting garbage rotation keys and filtering them out
                sanitizeAndFilterRotationKeys(animation, anim_channel, rotationKeys);

                for (uint32_t sKey = 0; sKey < anim_channel->mNumScalingKeys; sKey++) {
                    auto &key = anim_channel->mScalingKeys[sKey];
                    scaleKeys.emplace_back(key.mTime, glm::vec3(
                                               key.mValue.x,
                                               key.mValue.y,
                                               key.mValue.z
                                           ));
                }

                if (positionKeys.empty()) {
                    // TODO -> proper logging!
                    std::cout << "No position keys after processing for channel: " << anim_channel->mNodeName.C_Str()
                            << std::endl;
                }
                if (rotationKeys.empty()) {
                    // TODO -> proper logging!
                    std::cout << "No rotation keys after processing for channel: " << anim_channel->mNodeName.C_Str()
                            << std::endl;
                }
                if (scaleKeys.empty()) {
                    // TODO -> proper logging!
                    std::cout << "No scale keys after processing for channel: " << anim_channel->mNodeName.C_Str()
                            << std::endl;
                }

                animationChannels[anim_channel->mNodeName.C_Str()] = AnimationChannel{
                    .nodeName = anim_channel->mNodeName.C_Str(),
                    .positionKeys = positionKeys,
                    .rotationKeys = rotationKeys,
                    .scaleKeys = scaleKeys,
                };
            }
        }

        std::vector<glm::mat4> obtainAnimationPoses(double timePointInSeconds,
                                                    std::unordered_map<std::string, AnimationChannel> &
                                                    animationChannels);

        [[nodiscard]] const SkeletonNode &getBoneNode(const std::string &name) const {
            const auto itr = std::ranges::find_if(linearizedSkeletonNodes, [&](auto &node) {
                return node.name == name;
            });
            if (itr == linearizedSkeletonNodes.end()) {
                // TODO -> better error handling, no throwing
                throw std::runtime_error("Bone node not found");
            }
            return *itr;
        }

        static glm::mat<4, 4, float> convert_ai_matrix_to_glm(aiMatrix4x4 assimpMatrix) {
            return glm::transpose(glm::mat4(
                {
                    assimpMatrix.a1, assimpMatrix.a2,
                    assimpMatrix.a3, assimpMatrix.a4
                },
                {
                    assimpMatrix.b1, assimpMatrix.b2,
                    assimpMatrix.b3, assimpMatrix.b4
                },
                {
                    assimpMatrix.c1, assimpMatrix.c2,
                    assimpMatrix.c3, assimpMatrix.c4
                },
                {
                    assimpMatrix.d1, assimpMatrix.d2,
                    assimpMatrix.d3, assimpMatrix.d4
                }
            ));
        }

        [[nodiscard]] std::vector<Bone> getLinearizedBones() const {
            return linearizedBones;
        }

        [[nodiscard]] std::unordered_map<std::string, uint32_t> getBoneNamesToIndicesMap() const {
            return boneNamesToIndices;
        }

        bool cacheDebugInfo = false;

        AnimationInfoCache cache = {};
        std::unordered_map<std::string, AnimationChannel> animationChannels = {};

    private:
        glm::mat4 rootTransform = glm::mat4(1.0f);
        glm::mat4 inverseRootTransform = glm::mat4(1.0f);

        bool linearizeNodeHierarchy(const std::vector<Bone> &globalBonesList) {
            // recursively build up list of linearized bones
            linearizedSkeletonNodes.clear();
            return recursiveLinearize(globalBonesList, skeletonRootNode, -1);
        }

        bool recursiveLinearize(const std::vector<Bone> &globalBonesList, const aiNode *node, int32_t parentIndex);

        static float calculate_interpolation_factor(double currentKeyframeTimePointInTicks, double startTime,
                                                    double endTime);


        std::vector<SkeletonNode> linearizedSkeletonNodes = {};

        // constructed from the linearized skeleton nodes list, this is used to construct our final bone poses;
        // NOTE-> this probably means we don't need to maintain the unordered global bones list after ctor
        std::vector<Bone> linearizedBones = {};
        std::unordered_map<std::string, uint32_t> boneNamesToIndices = {};

        // one thing we may want to do: prefilter and store each node's animation keys,
        // to ensure that any funky assimp stuff has been at least spotted at import time
        double lastTime = 0.0;

        glm::vec3 computeInterpolatedPosition(AnimationChannel &node_anim,
                                              double currentKeyframeTimePointInTicks);

        glm::quat computeInterpolatedRotation(AnimationChannel &node_anim,
                                              double currentKeyframeTimePointInTicks);

        glm::vec3 computeInterpolatedScaling(AnimationChannel &node_anim,
                                             double currentKeyframeTimePointInTicks);


        static std::vector<AnimationChannel::RotationKey> cleanRotationKeys(
            const std::vector<AnimationChannel::RotationKey> &originalKeys) {
            auto keys = originalKeys;

            // Pass 1: Remove temporal duplicates and malformed time data
            keys = removeDuplicateRotations(keys);

            // Pass 2: Remove discontinuous jumps
            keys = filterDiscontinuousRotationKeys(keys);

            // Pass 3: Apply smoothness filter (disabled for now)
            // keys = filterByTemporalSmoothness(keys);

            return keys;
        }

        static std::vector<AnimationChannel::RotationKey> filterDiscontinuousRotationKeys(
            const std::vector<AnimationChannel::RotationKey> &originalKeys) {
            if (originalKeys.size() <= 2) return originalKeys;

            std::vector<AnimationChannel::RotationKey> filteredKeys;
            constexpr float MAX_ANGULAR_JUMP = glm::radians(120.0f);

            filteredKeys.push_back(originalKeys[0]); // Always keep first key

            for (size_t i = 1; i < originalKeys.size() - 1; i++) {
                const auto &prev = filteredKeys.back();
                const auto &current = originalKeys[i];
                const auto &next = originalKeys[i + 1];

                // Calculate angular distance to previous and next keyframes
                float angleToPrev = glm::angle(current.rotation * glm::inverse(prev.rotation));
                float angleToNext = glm::angle(next.rotation * glm::inverse(current.rotation));

                // If this keyframe creates a large jump, skip it
                if (angleToPrev > MAX_ANGULAR_JUMP || angleToNext > MAX_ANGULAR_JUMP) {
                    // Also check if removing this keyframe creates a smoother transition
                    float directAngle = glm::angle(next.rotation * glm::inverse(prev.rotation));
                    if (directAngle < angleToPrev + angleToNext) {
                        continue; // Skip this discontinuous keyframe
                    }
                }

                filteredKeys.push_back(current);
            }

            filteredKeys.push_back(originalKeys.back()); // Always keep last key
            return filteredKeys;
        }


        static std::vector<AnimationChannel::RotationKey> filterByTemporalSmoothness(
            const std::vector<AnimationChannel::RotationKey> &originalKeys) {
            if (originalKeys.size() <= 2) return originalKeys;

            std::vector<AnimationChannel::RotationKey> filteredKeys;

            filteredKeys.push_back(originalKeys[0]);

            for (size_t i = 1; i < originalKeys.size() - 1; i++) {
                const auto &prev = filteredKeys.back();
                const auto &current = originalKeys[i];
                const auto &next = originalKeys[i + 1];

                // Calculate "acceleration" in rotation space
                float timeDelta1 = current.time - prev.time;
                float timeDelta2 = next.time - current.time;

                if (timeDelta1 <= 0 || timeDelta2 <= 0) continue; // Skip malformed time data

                float angularVel1 = glm::angle(current.rotation * glm::inverse(prev.rotation)) / timeDelta1;
                float angularVel2 = glm::angle(next.rotation * glm::inverse(current.rotation)) / timeDelta2;

                float angularAccel = std::abs(angularVel2 - angularVel1) / ((timeDelta1 + timeDelta2) * 0.5f);

                // If angular acceleration is too high, this might be a corrupt keyframe
                if (constexpr float SMOOTHNESS_THRESHOLD = 2.0f; angularAccel > SMOOTHNESS_THRESHOLD) {
                    continue;
                }

                filteredKeys.push_back(current);
            }

            filteredKeys.push_back(originalKeys.back());
            return filteredKeys;
        }

        static std::vector<AnimationChannel::RotationKey> removeDuplicateRotations(
            const std::vector<AnimationChannel::RotationKey> &originalKeys) {
            if (originalKeys.size() <= 1) return originalKeys;

            std::vector<AnimationChannel::RotationKey> filteredKeys;
            constexpr float MIN_ROTATION_CHANGE = 0.01f; // Minimum quaternion dot product difference

            filteredKeys.push_back(originalKeys[0]);

            for (size_t i = 1; i < originalKeys.size(); i++) {
                const auto &last = filteredKeys.back();
                const auto &current = originalKeys[i];

                // Check if rotations are too similar (using dot product)
                float similarity = std::abs(glm::dot(last.rotation, current.rotation));

                // If rotations are essentially the same, skip this keyframe
                if (similarity > (1.0f - MIN_ROTATION_CHANGE)) {
                    continue;
                }
                filteredKeys.push_back(current);
            }

            return filteredKeys;
        }

        void sanitizeAndFilterRotationKeys(aiAnimation *animation, aiNodeAnim *&anim_channel,
                                           std::vector<AnimationChannel::RotationKey> &rotationKeys) {
            // NEW IDEA FOR SOMETHING TO TRY:
            // I think an extremely simple sanitization method for specifically looping animations is that
            // we can hopefully just grab the last valid keyframe rotation and pull it forward to t=0;
            // all other keys whose time values are less than 1 will be dropped and forgotten about because
            // they're probably junk anyway and there's pretty much no reliable way to sift through them easily.
            static constexpr uint32_t MAX_COUNT_BEFORE_CONSIDERED_OUTLIER = 2;
            static constexpr double QUAT_COMPARE_EPSILON = 0.001;
            std::function compareQuaternions
                    = [&](const aiQuaternion &a, const aiQuaternion &b) {
                return std::abs(a.w - b.w) < QUAT_COMPARE_EPSILON && std::abs(a.x - b.x) < QUAT_COMPARE_EPSILON
                       && std::abs(a.y - b.y) < QUAT_COMPARE_EPSILON && std::abs(a.z - b.z) < QUAT_COMPARE_EPSILON;
            };
            // load all of them keys up and then pre-sort them to bring all the interspersed t<1 values to the front
            // NEW: attempt to filter out strange duplicate keys that appear early on and seem to break everything
            std::set<double> timePointsEncountered = {};
            std::vector<aiQuatKey> sortedRotationKeysWithoutDuplicates = {};
            for (uint32_t rotKey = 0; rotKey < anim_channel->mNumRotationKeys; rotKey++) {
                aiQuatKey key = anim_channel->mRotationKeys[rotKey];

                // ASSUMPTION WHICH MIGHT BE WRONG: first time key we encounter of a given time will be correct
                // NOTE -> we need a valid anim duration and ticks/second value here!
                if (double time = key.mTime;
                    time >= 1 && !timePointsEncountered.contains(time) && time <= animation->mDuration) {
                    sortedRotationKeysWithoutDuplicates.emplace_back(key);
                    timePointsEncountered.insert(time);
                }
            }
            // sort resulting keys
            std::ranges::sort(sortedRotationKeysWithoutDuplicates,
                              [&](const aiQuatKey &a, const aiQuatKey &b) {
                                  return a.mTime < b.mTime;
                              });
            std::vector<aiQuatKey> filteredSortedRotationKeysWithoutDuplicates = {};
            std::vector<aiQuaternion> blacklist = {};

            for (auto &key: sortedRotationKeysWithoutDuplicates) {
                // new idea: look at each key; if it has too many appearances, delete it.
                // not the most robust way of looking for outliers but hopefully it works for now.
                auto itr = std::ranges::find_if(blacklist, [&](const aiQuaternion &outlierKey) {
                    return compareQuaternions(outlierKey, key.mValue);
                });
                if (itr != blacklist.end()) {
                    continue;
                }

                // could optimize this with a whitelist but for now i'll keep it inefficient

                // in this case we have an unencountered quaternion key, and so we have to check to see how many
                // appearances it has
                uint32_t keyCount = 0;
                for (auto &k: sortedRotationKeysWithoutDuplicates) {
                    if (compareQuaternions(k.mValue, key.mValue)) keyCount++;
                }

                if (keyCount > MAX_COUNT_BEFORE_CONSIDERED_OUTLIER) {
                    blacklist.emplace_back(key.mValue);
                    continue;
                }

                filteredSortedRotationKeysWithoutDuplicates.emplace_back(key);
            }

            if (!filteredSortedRotationKeysWithoutDuplicates.empty()) {
                // find first valid rotation key (which we deem as the first one whose time point is >= 1)
                uint32_t firstValidKeyIndex = 0;
                for (uint32_t rotKey = 0; rotKey < filteredSortedRotationKeysWithoutDuplicates.size(); rotKey++) {
                    if (filteredSortedRotationKeysWithoutDuplicates[rotKey].mTime >= 1) {
                        // we want the first frame on any tick but 0
                        firstValidKeyIndex = rotKey;
                        break;
                    }
                }

                // FOR LOOPING ANIMATIONS (which can be made specifiable but will be assumed for now)
                // acquire the last keyframe and use it for the first.
                // this is technically mangling it but it has to be done otherwise we're stuck with junk that ruins
                // our interp
                const aiQuatKey lastValidKey
                        = filteredSortedRotationKeysWithoutDuplicates[
                            filteredSortedRotationKeysWithoutDuplicates.size() - 1];
                rotationKeys.emplace_back(0, glm::normalize(glm::quat(
                                              lastValidKey.mValue.w,
                                              lastValidKey.mValue.x,
                                              lastValidKey.mValue.y,
                                              lastValidKey.mValue.z
                                          )));

                for (uint32_t rotKey = firstValidKeyIndex;
                     rotKey < filteredSortedRotationKeysWithoutDuplicates.size();
                     rotKey++) {
                    auto &key = filteredSortedRotationKeysWithoutDuplicates[rotKey];

                    // just in case
                    if (key.mTime < 1) {
                        continue;
                    }

                    // skip any keyframes whose times are beyond our valid range (NOTE: reqs valid ticks per second value)
                    if (animation->mTicksPerSecond != 0
                        && key.mTime > animation->mDuration) {
                        continue;
                    }

                    rotationKeys.emplace_back(key.mTime, glm::normalize(glm::quat(
                                                  key.mValue.w,
                                                  key.mValue.x,
                                                  key.mValue.y,
                                                  key.mValue.z
                                              )));
                }
            } else {
                // in the case where we have *only* duplicate keys, as a hacky measure, just take the back of the blacklist
                // as the only key (this will be removed when spline interp is added in later)
                if (!blacklist.empty())
                    rotationKeys = {
                        {
                            0.0,
                            glm::normalize(glm::quat(blacklist.back().w, blacklist.back().x, blacklist.back().y,
                                                     blacklist.back().z))
                        }
                    };
                else if (anim_channel->mNumRotationKeys > 0) {
                    auto key = anim_channel->mRotationKeys[0];
                    rotationKeys = {
                        {
                            key.mTime,
                            glm::normalize(glm::quat(
                                key.mValue.w,
                                key.mValue.x,
                                key.mValue.y,
                                key.mValue.z
                            ))
                        }
                    };
                } else {
                    rotationKeys = {
                        {
                            0.0,
                            glm::normalize(glm::quat())
                        }
                    };
                }
            }
        }
    };
}
