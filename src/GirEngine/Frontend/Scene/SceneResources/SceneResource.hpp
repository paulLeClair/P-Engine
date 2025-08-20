//
// Created by paull on 2022-06-14.
//

#pragma once

#include <memory>

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::girEngine::gir {
    class GraphicsIntermediateRepresentation;
}

namespace pEngine::girEngine::scene {

    /**
     * TODO -> evaluate ripping this out; inheritance in this case seems 1000% pointless
     */
    class Resource {
    public:
        struct CreationInput {
            std::string name;

            util::UniqueIdentifier uid;
        };

        explicit Resource(const CreationInput &creationInput)
                : name(creationInput.name),
                  uid(creationInput.uid) {

        }

        virtual ~Resource() = default;

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uid;
        }

        virtual std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() = 0;

        enum class UpdateResult {
            SUCCESS,
            FAILURE
        };

        /**
         * TODO - hash out all the update stuff once we get static scenes rendering
         */
        virtual UpdateResult update() = 0;

    private:
        std::string name;

        util::UniqueIdentifier uid;

    };

}