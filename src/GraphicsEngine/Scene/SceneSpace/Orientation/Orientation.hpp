//
// Created by paull on 2024-09-11.
//

#pragma once

/**
 * This is a placeholder class for representing the orientation of an object in the scene;
 * for now it'll be ultra basic for the purposes of positioning the demo model in front of the camera,
 * supporting only whatever is required to rotate the model so that it's facing the camera.
 */

#include <glm/gtc/quaternion.hpp>

namespace pEngine::girEngine::scene::space::orient {
    class Orientation {
    public:
        explicit Orientation(const glm::quat orientation) : orientation(orientation) {
        }

        Orientation(float pitch, float yaw, float roll) {
        }

        Orientation() {
        }

        // TODO - functions for computing the rotation matrix etc if convenient

        const glm::quat &getOrientation() const {
            return orientation;
        }

        [[nodiscard]] glm::mat4 getOrientationMatrix() const {
            return mat4_cast(orientation);
        }

    private:
        /**
         * This orientation stuff is going to need a lot of fleshing out; my idea is to just have this class wrap up
         * some kind of optimized representation and then provide a few handy functions for getting different
         * orientation-related quantities; to start I'm going to keep it very simple though
         *
         */

        glm::vec3 lookDirection = {};

        glm::quat orientation{};
    };
}
