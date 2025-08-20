//
// Created by paull on 2024-06-23.
//

#pragma once


#include <glm/vec3.hpp>

namespace pEngine::girEngine::scene::space::velocity {
    class Velocity {
    public:
        struct CreationInput {
            double xVelocity = 0.0f;
            double yVelocity = 0.0f;
            double zVelocity = 0.0f;
        };

        explicit Velocity(const CreationInput &creationInput) : velocity(creationInput.xVelocity,
                                                                         creationInput.yVelocity,
                                                                         creationInput.zVelocity) {
        }

        Velocity() : velocity() {
        }

        glm::vec3 &getVelocity() {
            return velocity;
        }

    private:
        glm::vec3 velocity;
    };
}
