//
// Created by paull on 2024-09-04.
//

#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../KeyframeIR/KeyframeIR.hpp"
#include <chrono>
#include <map>

namespace pEngine::girEngine::scene::model::anim {
    class Bone;
}

namespace pEngine::girEngine::gir::model {
    class AnimationIR final : public GraphicsIntermediateRepresentation {
    public:
        struct CreationInput {
            std::string name;

            // so here we'll probably want to just model the IR fields after what we're building up
            // in the scene animations; I might want to break off a little class that basically
            // generates keyframes that kinda ports out some of the stuff that's in the model now;
            // the idea being that all it's doing is just interpolating between a set of keyframe matrix sets
            // according to some defined animation length

            // on the other hand, to achieve the whole "intermediate layer" goal with this GIR shite,
            // it might make more sense to just package the animation data as keyframes and that's it;
            // we can leave the actual handling / processing of the animation for the backend.
            // this way we can add different animation GIR (sub?)classes to allow different ways to
            // package the animation data, eg as the assimp-style channels (which would give a lot more fidelity
            // since I'm basically just sampling the data a few times for now (which may not even end up working lol))

            std::vector<KeyframeIR> keyframes = {};

            double animationTicksPerSecond;
            double animationDurationInSeconds;
        };

        explicit AnimationIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation({
                  creationInput.name,
                  util::UniqueIdentifier(),
                  GIRSubtype::ANIMATION
              }),
              animationDurationInSeconds(creationInput.animationDurationInSeconds),
              keyframes(creationInput.keyframes),
              animationTicksPerSecond(creationInput.animationTicksPerSecond) {
        }

        double animationDurationInSeconds = 1.0;

        boost::optional<std::chrono::seconds> animationStartTime = boost::none;

        std::vector<KeyframeIR> keyframes = {};

        double animationTicksPerSecond;

        double getAnimationDurationInSeconds() const {
            return animationDurationInSeconds;
        }

        [[nodiscard]] boost::optional<std::chrono::seconds> getAnimationStartTime() const {
            return animationStartTime;
        }
    };
}
