//
// Created by paull on 2022-06-14.
//

// DEPRECATED

#pragma once

#include "../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"

/**
 * New idea: this SceneResource class denotes all individual scene elements that can be thought of as resources
 * in the traditional rendering sense (ie various forms of buffers and images)\n \n
 *
 * Buffer -> should be a general purpose buffer wrapper class\n \n
 *
 * Model -> should be a conveniently-designed wrapper for collections of geometry data (which need their own buffers),
 * plus an easy way to hook in any other resources that might be used in rendering the model (eg materials) \n \n
 *
 * Image -> should be a general purpose image wrapper class\n \n
 *
 * Material -> should be a conveniently-designed wrapper for related material-specific data (mostly images and maybe some specialized buffers for certain shader techniques), eg textures and other
 * bits of data that are tied to some model\n \n
 *
 * Texture (?) -> TODO, might make sense to provide a separate category in case you want to use textures but don't need the extra functionality / overhead of a Material,
 * and then Materials could be based around these (can be left as todo for now) \n \n
 *
 * From here, we may need to add more, so this list is subject to change!
 *
 */

class Scene;

class SceneResource {
public:
    virtual ~SceneResource() = default;

    [[nodiscard]] virtual const std::string &getName() const = 0;

    [[nodiscard]] virtual const PUtilities::UniqueIdentifier &getUniqueIdentifier() const = 0;

    [[nodiscard]] virtual const std::shared_ptr<Scene> &getParentScene() const = 0;

    enum class UpdateResult {
        SUCCESS,
        FAILURE
    };

    /**
     * Okay, so if we want to avoid passing in some sort of update data directly,
     * I think the idea is to expect that the update callback that the user provides the model
     * will actually copy the new data (or otherwise compute it) into the SceneResource itself,
     * and then after that it will be ready to commit the updated data that it's storing into the
     * various appropriate backend objects. \n \n
     *
     * I'm still not sure what form committing updates will take - since we have to copy data to/(from!) the GPU
     * it might require submitting some commands and using a fence or some other synchronization thing so that drawing commands are executed
     * only once all data transfer is complete.
     * @return An indicator of what happened so that the caller can handle appropriately (might get rid of the enum and make it void if this proves unnecessary)
     */
    virtual UpdateResult update() = 0;

    // TODO - factor out name, uid, and parent scene into protected members of this class
    // so it's not pure abstract

};