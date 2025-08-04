//
// Created by paull on 2025-02-07.
//

#pragma once

#include <string>

#include <glm/glm.hpp>

namespace pEngine::girEngine::gir::light {
    /**
     * This will not be a GIR subclass (I might even just remove the notion of a gir subclass anyway)
     */
    struct PointLightIR {
        std::string name;
        UniqueIdentifier identifier;

        glm::vec3 position;
        glm::vec3 color;
    };
}
