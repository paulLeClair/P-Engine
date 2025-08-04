//
// Created by paull on 2024-06-23.
//

#pragma once


#include <glm/vec3.hpp>

namespace pEngine::girEngine::scene::space::position {
    // COMING BACK TO THIS -> TODO: kill all of this with fire

    /**
     * I guess there's 2 ways I could take this class:
     *
     * 1. have it be something that uses class composition to give classes a position;
     * 2. have it be used almost internally to the space so that objects can register and be given a position object
     * that they can use (and the velocity class would work the same way)
     *
     * I think number 2 seems a lot nicer; less coupling and what not (at least it seems that way to me)
     */
    class Position {
    public:
        struct CreationInput {
            double xCoordinate = 0.0f;
            double yCoordinate = 0.0f;
            double zCoordinate = 0.0f;
        };

        explicit Position(const CreationInput &creationInput) : position(creationInput.xCoordinate,
                                                                         creationInput.yCoordinate,
                                                                         creationInput.zCoordinate) {
        }

        Position() : position() {
        }

        [[nodiscard]] glm::vec3 &getPosition() {
            return position;
        }


        glm::vec3 position;
    };
}
