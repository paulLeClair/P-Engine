//
// Created by paull on 2024-06-25.
//

#pragma once


#include "../GraphicsIntermediateRepresentation.hpp"

#include <glm/glm.hpp>

namespace pEngine::girEngine::gir::camera {
    /**
     * TODO -> either rename everything to use this GIR suffix or
     * change this one to use IR
     */
    class CameraGIR : public gir::GraphicsIntermediateRepresentation {
    public:
        struct CreationInput : gir::GraphicsIntermediateRepresentation::CreationInput {
            // for now, just worldspace position, velocity, and orientation
            glm::vec3 &position;
            glm::vec3 &velocity;

            double &pitch;
            double &yaw;

            double verticalFieldOfView;

            double aspectRatio;

            double nearPlaneDistance;

            double farPlaneDistance;
        };

        explicit CameraGIR(const CreationInput &input) : gir::GraphicsIntermediateRepresentation(input),
                                                         position(input.position),
                                                         velocity(input.velocity),
                                                         pitch(input.pitch),
                                                         yaw(input.yaw),
                                                         verticalFieldOfView(input.verticalFieldOfView),
                                                         aspectRatio(input.aspectRatio),
                                                         nearPlaneDistance(input.nearPlaneDistance),
                                                         farPlaneDistance(input.farPlaneDistance) {
        }

    private:
        glm::vec3 &position;
        glm::vec3 &velocity; // maybe unused

        double &pitch;
        double &yaw;

        double verticalFieldOfView;

        double aspectRatio;

        double nearPlaneDistance;

        double farPlaneDistance;
    };
}
