//
// Created by paull on 2024-09-08.
//

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::model {
    class KeyframeIR : public GraphicsIntermediateRepresentation {
    public:
        struct CreationInput {
            std::string name;
            long long timePointInAnimationTicks = 0;

            std::vector<glm::mat4> poses = {};
        };

        explicit KeyframeIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(
                  {
                      creationInput.name,
                      util::UniqueIdentifier(),
                      GIRSubtype::KEYFRAME
                  }),
              timePointInAnimationTicks(creationInput.timePointInAnimationTicks), poses(creationInput.poses) {
        }

        [[nodiscard]] double getTimePoint() const {
            return timePointInAnimationTicks;
        }

        [[nodiscard]] const std::vector<glm::mat4> &getPoses() const {
            return poses;
        }

    private:
        // NOTE -> this doesn't support negative time points, you'll have to shift the time domain to be positive
        double timePointInAnimationTicks = 0;

        std::vector<glm::mat4> poses = {};
    };
}
