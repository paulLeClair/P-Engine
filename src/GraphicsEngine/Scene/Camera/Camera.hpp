//
// Created by paull on 2023-01-07.
//

#pragma once

#include <memory>
#include <stdexcept>
#include <boost/math/constants/constants.hpp>

#include <glm/detail/type_quat.hpp>
#include "../SceneSpace/Position/Position.hpp"
#include "../SceneSpace/Velocity/Velocity.hpp"
#include "../../GraphicsIR/CameraGIR/CameraGIR.hpp"

namespace pEngine::girEngine::gir {
    class GraphicsIntermediateRepresentation;
}

namespace pEngine::girEngine::scene::view {
    /**
     * Very much TODO until we're ready to get an actual 3D scene with models rendered
     */
    class Camera {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;

            space::position::Position position;
            space::velocity::Velocity velocity;

            double pitch = 0.0f;
            double yaw = 0.0f;

            double verticalFieldOfView = 80.0f / boost::math::constants::pi<double>();

            /**
             * The aspect ratio is just the width / height; used for computation of perspective matrix
             */
            double aspectRatio;

            double nearPlaneDistance;

            double farPlaneDistance;
        };

        explicit Camera(const CreationInput &creationInput)
            : name(creationInput.name),
              uid(creationInput.uid),
              position(creationInput.position),
              velocity(creationInput.velocity),
              pitch(creationInput.pitch),
              yaw(creationInput.yaw),
              verticalFieldOfView(creationInput.verticalFieldOfView),
              aspectRatio(creationInput.aspectRatio),
              nearPlaneDistance(creationInput.nearPlaneDistance),
              farPlaneDistance(creationInput.farPlaneDistance) {
        }

        ~Camera() = default;

        /**
         * We can probably start thinking about how to implement viewport stuff now!
         *
         * The idea with this thing is to basically provide a convenient way to control the camera -
         * it should probably be somewhat similar to the scene resources and make use of a simple "update" interface
         * type thing so that the user can do whatever they want with it to update the data that goes with it.
         *
         *
         */

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uid;
        }

        gir::camera::CameraGIR bakeToGIR() {
            return gir::camera::CameraGIR(gir::camera::CameraGIR::CreationInput{
                name,
                uid,
                gir::GIRSubtype::CAMERA,
                position.getPosition(),
                velocity.getVelocity(),
                pitch,
                yaw,
                verticalFieldOfView,
                aspectRatio
            });
        }

        // idea for now: you just get references to these things that you can change externally

        [[nodiscard]] space::position::Position &getPosition() {
            return position;
        }

        [[nodiscard]] space::velocity::Velocity &getVelocity() {
            return velocity;
        }

        [[nodiscard]] double &getPitch() {
            return pitch;
        }

        [[nodiscard]] double &getYaw() {
            return yaw;
        }

        static glm::mat4 computeViewTransform(
            const glm::vec3 &position,
            const float yaw,
            const float pitch
        ) {
            auto result = translate(glm::mat4(1.0f), -position);
            result = rotate(result, -yaw, glm::vec3(0, 0, 1));
            result = rotate(result, -pitch, glm::vec3(0, 1, 0));

            return result;
        }

        static glm::mat4 computePerspectiveTransform(
            const float verticalFieldOfView,
            const float aspectRatio,
            const float nearPlaneDistance,
            const float farPlaneDistance
        ) {
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
            auto result = glm::perspectiveLH(verticalFieldOfView,
                                             aspectRatio,
                                             nearPlaneDistance,
                                             farPlaneDistance);
#undef GLM_FORCE_DEPTH_ZERO_TO_ONE

            // NEW ASSUMPTION: since glm::mat4[] returns column major, we can just use result
            return result;
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        // gonna try making these references since they should be acquired from the SceneSpace (ATOW)
        space::position::Position position;
        space::velocity::Velocity velocity;

        // there are a couple reasons not to use quaternions for camera rotation (from RTR textbook, I'll find the specific reasoning and put it here)
        // and it's also a bit easier as far as the logic goes to disallow the camera to roll
        double pitch = 0.0f;
        double yaw = 0.0f;

        // just realized: one of the more important purposes of the camera is to define the view volume;
        // therefore we'll probably want some near and far planes, which I think are usually obtained
        // using a FOV angle and aspect ratio
        double verticalFieldOfView = 0.0f;

        /**
          * The aspect ratio is just the width / height; used for computation of perspective matrix
         */
        double aspectRatio;

        double nearPlaneDistance;

        double farPlaneDistance;
    };
}
