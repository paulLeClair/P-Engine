//
// Created by paull on 2024-06-19.
//

#pragma once

#include <string>
#include "Position/Position.hpp"
#include "Velocity/Velocity.hpp"
#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include <btBulletDynamicsCommon.h>

// COMING BACK TO THIS -> not sure how I wanna structure all this scene space/physics stuff yet;
// consider all of this to be TBD and very much subject to change

namespace pEngine::girEngine::scene::space {

    enum class SpaceVolumeOptions { // struggling with naming these; I'll try an "options" thing and see how that feels
        UNKNOWN,
        INFINITE_VOLUME,
        FINITE_VOLUME
    };

    /**
     * The design for this one I'm still not sure about...
     *
     * Maybe I can treat it as a thing where you basically just register all "objects" that you're
     * placing in your scene, and the scene can kinda facilitate (or otherwise is a part of) the process of
     * updating all the scene objects in a clean way
     */
    class SceneSpace {
    public:

        struct CreationInput {
            SpaceVolumeOptions boundaryType = SpaceVolumeOptions::UNKNOWN;

        };

        explicit SceneSpace(const CreationInput &creationInput)
                : boundaryType(creationInput.boundaryType) {

        }

        ~SceneSpace() = default;


        // ew i don't like this anymore :'(
        space::position::Position &requestPositionHandle(const util::UniqueIdentifier positionIdentifier) {
            if (positions.find(positionIdentifier) == positions.end()) {
                // if we don't find the position, enqueue a new one
                positions.insert(
                        {positionIdentifier,
                         space::position::Position(space::position::Position::CreationInput{
                                 0.0,
                                 0.0,
                                 0.0
                         })
                        }
                );
            }

            return positions.at(positionIdentifier);
        }

        // ew this is gross :'( get rid of it
        space::velocity::Velocity &requestVelocityHandle(const util::UniqueIdentifier &velocityIdentifier) {
            if (velocities.find(velocityIdentifier) == velocities.end()) {
                velocities.insert(
                        {
                                velocityIdentifier,
                                space::velocity::Velocity(space::velocity::Velocity::CreationInput{
                                        0.0,
                                        0.0,
                                        0.0
                                })
                        }
                );
            }

            return velocities.at(velocityIdentifier);
        }

    private:
        /**
         * Still figuring out what needs to be put in here...
         *
         * In the future the internals of this class might get ripped out and replaced.
         *
         * The idea I have right now for a simple first pass is a Struct-of-Arrays design
         * where we make a class that gives any inheritor the ability to store its position & velocity
         *
         * Cache-wise I'm not sure what's more optimal, to store the position & velocity together or separate,
         * but it's simpler (ish) to just keep em separate for now. Easy enough to adjust anyway.
         *
         *
         */

        // space type (not sure if this will be necessary long term lol)
        const SpaceVolumeOptions boundaryType = SpaceVolumeOptions::UNKNOWN;

        // positions vector;
        std::unordered_map<util::UniqueIdentifier, space::position::Position> positions = {};

        // velocities
        std::unordered_map<util::UniqueIdentifier, space::velocity::Velocity> velocities = {};

    };

}