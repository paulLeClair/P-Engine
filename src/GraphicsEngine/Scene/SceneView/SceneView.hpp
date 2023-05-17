//
// Created by paull on 2023-01-07.
//

#pragma once

#include <memory>
#include "../Scene.hpp"

class SceneView {
public:
    virtual ~SceneView() = default;

    /**
     * We can probably start thinking about how to implement viewport stuff now!
     *
     * The idea with this thing is to basically provide a convenient way to control the camera -
     * it should probably be somewhat similar to the scene resources and make use of a simple "update" interface
     * type thing so that the user can do whatever they want with it to update the data that goes with it.
     *
     *
     */

    [[nodiscard]] virtual const std::shared_ptr<Scene> &getParentScene() const = 0;

    [[nodiscard]] virtual const std::string &getName() const = 0;

    [[nodiscard]] virtual const PUtilities::UniqueIdentifier &getUniqueIdentifier() const = 0;


};