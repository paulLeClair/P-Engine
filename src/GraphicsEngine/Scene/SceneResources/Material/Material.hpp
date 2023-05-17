//
// Created by paull on 2022-07-17.
//

#pragma once

#include <memory>

#include "../SceneResource.hpp"
//#include "../../Scene.hpp"
//#include "../Texture/Texture.hpp"
//#include "../Buffer/Buffer.hpp"
//#include "../ShaderConstant/ShaderConstant.hpp"
#include "../Renderable/Renderable.hpp"

using namespace PUtilities;

/**
 * Still not 100% clear on everything that's needed for a good simple material system, but since most materials
 * (datawise) boil down to collections of textures that are all used in some fancy way on the GPU, it might make sense
 * to have them simply be collections of logically-related textures
 *
 * Hopefully that allows for everything from simple "slap some texels on it" materials to full PBR materials with
 * albedo, roughness, and bumpmap textures (or whatever)
 *
 * I'll also add in the ability to associate any buffer or image that you want with a material, being that those are
 * the basic building block types - hopefully this should allow you to come up with any number of ways to use material
 * textures
 */

class Buffer;

class Texture;

class Image;

class ShaderConstant;

class Renderable;

class Material : public SceneResource {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;
        std::string name;
        PUtilities::UniqueIdentifier uid;

        std::vector<std::shared_ptr<Texture>> initialTextures;
        std::vector<std::shared_ptr<Buffer>> initialBuffers;
        std::vector<std::shared_ptr<Image>> initialImages;
        std::vector<std::shared_ptr<ShaderConstant>> initialShaderConstants;

        // TODO - evaluate whether Materials could be able to have their own renderables? kinda makes sense
        std::vector<std::shared_ptr<Renderable>> initialRenderables = {};
    };

    explicit Material(const CreationInput &creationInput) {

    }

    ~Material() override = default;

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uid;
    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &getTextures() const {
        return textures;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getBuffers() const {
        return buffers;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Image>> &getImages() const {
        return images;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<ShaderConstant>> &getShaderConstants() const {
        return shaderConstants;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Renderable>> &getRenderables() const {
        return renderables;
    }

    UpdateResult update() override {
        return UpdateResult::FAILURE;
    }

private:
    std::shared_ptr<Scene> parentScene;
    std::string name;
    PUtilities::UniqueIdentifier uid;

    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<std::shared_ptr<Buffer>> buffers;
    std::vector<std::shared_ptr<Image>> images;
    std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;

    // TODO - evaluate whether Materials could be able to have their own renderables? kinda makes sense
    std::vector<std::shared_ptr<Renderable>> renderables = {};


};
