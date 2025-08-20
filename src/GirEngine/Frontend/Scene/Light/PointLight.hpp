//
// Created by paull on 2025-02-07.
//

#pragma once

#include <string>

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../SceneSpace/Position/Position.hpp"

namespace pEngine::girEngine::scene::light {
    struct PointLight {
        std::string name;
        util::UniqueIdentifier identifier;

        space::position::Position position;
        glm::vec3 color;
    };
}
