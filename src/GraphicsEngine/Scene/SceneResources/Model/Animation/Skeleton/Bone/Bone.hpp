//
// Created by paull on 2024-07-13.
//

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "../../../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../ModelSegment/ModelSegment.hpp"

namespace pEngine::girEngine::scene::model::anim {
    /**
     * For the assimp integration, this should basically be a light wrapper over the aiBone abstraction
     * and this is probably where we'll migrate the "transform" calculation stuff (which currently lives
     * in the model segment thing)
     *
     *
     * We need to:
     * - store the bone's combined transform matrix here, so it can be copied into a device memory buffer for access in the vert shader
     * - (externally to this class) copy in the weights (and indices of matrices in the device memory matrix array) to the vertex buffer
     *      -> actually maybe this class can facilitate the second point here by storing a wrapped version of assimp's vertex weights,
     *      and then we could just iterate over these to actually populate the per-vertex data when we're filling the vertex buffer
     *
     * -> coming back to this, it seems like we are not using the combined/accumulated matrix here at all;
     * I'm not sure whether that's even wrong right now, but it's possible that I need to be storing the "bind pose"
     * transforms to be using to determine relative transformations for the vertices from the absolute transformations that
     * we're given in the animation channel keys (by differencing them or w/e)
     * -> it seems like we aren't storing certain bone info here because we're just using that info in a few spots
     * where it's needed by other classes;
     */
    class Bone {
    public:
        struct VertexWeight {
            float weight;
            size_t affectedVertex; // index into the global vertices array for this model
        };

        struct CreationInput {
            std::string name{};
            util::UniqueIdentifier uniqueIdentifier{};

            std::vector<VertexWeight> vertexWeights{}; // LIKELY DEPRECATED

            glm::mat4 inverseBindPose{};
        };

        explicit Bone(const CreationInput &creationInput) : name(creationInput.name),
                                                            uniqueIdentifier(creationInput.uniqueIdentifier),
                                                            inverseBindPose(creationInput.inverseBindPose),
                                                            vertexWeights(creationInput.vertexWeights) {
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUniqueIdentifier() const {
            return uniqueIdentifier;
        }

        [[nodiscard]] const std::vector<VertexWeight> &getVertexWeights() const {
            return vertexWeights;
        }

        [[nodiscard]] const glm::mat4 &getInverseBindPose() const {
            return inverseBindPose;
        }

    private:
        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        glm::mat4 inverseBindPose = {};

        std::vector<VertexWeight> vertexWeights;
    };
}
