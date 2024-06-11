//
// Created by paull on 2023-01-07.
//

#pragma once

#include <memory>
#include <stdexcept>
#include "../../../lib/glm/detail/type_quat.hpp"

namespace pEngine::girEngine::gir {
    class GraphicsIntermediateRepresentation;
}

namespace pEngine::girEngine::scene {

    /**
     * Very much TODO until we're ready to get an actual 3D scene with models rendered
     */
    class View {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
        };

        explicit View(const CreationInput &creationInput)
                : name(creationInput.name),
                  uid(creationInput.uid),
                  position(glm::vec3(0.0f)),
                  rotationQuaternion(glm::qua<float, glm::precision::highp>(0.0f, 0.0f, 0.0f, 0.0f)) {

        }

        ~View() = default;

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

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() {
            throw std::runtime_error(
                    "Error in scene::View::bakeToGIR() for View "
                    + name + ": functionality not yet implemented!");
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        glm::vec3 position; // TODO - evaluate what to do for this position representation

        glm::qua<double, glm::precision::highp> rotationQuaternion;
    };

}