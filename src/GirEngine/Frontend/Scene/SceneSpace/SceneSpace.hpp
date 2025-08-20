//
// Created by paull on 2024-06-19.
//

#pragma once

#include "Position/Position.hpp"
#include "Velocity/Velocity.hpp"
#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

// #include <btBulletDynamicsCommon.h>

namespace pEngine::girEngine::scene::space {
    enum class SpaceVolumeOptions {
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

        SceneSpace(const SceneSpace &other)
            : boundaryType(other.boundaryType),
              positions(other.positions),
              velocities(other.velocities) {
        }

        SceneSpace(SceneSpace &&other) noexcept
            : boundaryType(other.boundaryType),
              positions(std::move(other.positions)),
              velocities(std::move(other.velocities)) {
        }

        SceneSpace & operator=(const SceneSpace &other) {
            if (this == &other)
                return *this;
            boundaryType = other.boundaryType;
            positions = other.positions;
            velocities = other.velocities;
            return *this;
        }

        SceneSpace & operator=(SceneSpace &&other) noexcept {
            if (this == &other)
                return *this;
            boundaryType = other.boundaryType;
            positions = std::move(other.positions);
            velocities = std::move(other.velocities);
            return *this;
        }

        position::Position &requestPositionHandle(const UniqueIdentifier positionIdentifier) {
            if (!positions.contains(positionIdentifier)) {
                // if we don't find the position, enqueue a new one
                positions.insert(
                    {
                        positionIdentifier,
                        position::Position(position::Position::CreationInput{
                            0.0,
                            0.0,
                            0.0
                        })
                    }
                );
            }

            return positions.at(positionIdentifier);
        }

        velocity::Velocity &requestVelocityHandle(const UniqueIdentifier &velocityIdentifier) {
            if (!velocities.contains(velocityIdentifier)) {
                velocities.insert(
                    {
                        velocityIdentifier,
                        velocity::Velocity(velocity::Velocity::CreationInput{
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
        SpaceVolumeOptions boundaryType = SpaceVolumeOptions::UNKNOWN;

        // positions vector;
        std::unordered_map<UniqueIdentifier, position::Position> positions = {};

        // velocities
        std::unordered_map<UniqueIdentifier, velocity::Velocity> velocities = {};
    };
}
