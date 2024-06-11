//
// Created by paull on 2022-07-17.
//

#pragma once

#include <memory>
#include <vector>

#include "../Buffer/Buffer.hpp"
#include "../Texture/Texture.hpp"
#include "../Renderable/Renderable.hpp"
#include "../ShaderConstant/ShaderConstant.hpp"

/**
 * Still not 100% clear on everything that's needed for a good simple material system, but since most materials
 * (datawise) boil down to collections of textures that are all used in some fancy way on the GPU, it might make sense
 * to have them simply be collections of logically-related textures
 *
 * Hopefully that allows for everything from simple "slap some texels on it" materials to full PBR/ray tracing materials
 * with albedo, roughness, and bump-map textures (or whatever)
 *
 * I'll also add in the ability to associate any buffer or image that you want with a material, being that those are
 * the basic building block types - hopefully this should allow you to come up with any number of ways to use material
 * textures
 */

namespace pEngine::girEngine::scene {

    class Material : public scene::Resource {
    public:
        struct CreationInput : public scene::Resource::CreationInput {

            std::vector<std::shared_ptr<scene::Buffer>> initialBuffers;
            std::vector<std::shared_ptr<scene::Image>> initialImages;
            std::vector<std::shared_ptr<scene::ShaderConstant>> initialShaderConstants;
            std::vector<std::shared_ptr<scene::Texture>> initialTextures;

            std::vector<std::shared_ptr<scene::Renderable>> initialRenderables = {};
        };

        explicit Material(const CreationInput &creationInput)
                : Resource(creationInput),
                  textures(creationInput.initialTextures),
                  buffers(creationInput.initialBuffers),
                  images(creationInput.initialImages),
                  shaderConstants(creationInput.initialShaderConstants),
                  renderables(creationInput.initialRenderables) {

        }

        ~Material() override = default;

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Texture>> &getTextures() const {
            return textures;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Buffer>> &getBuffers() const {
            return buffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Image>> &getImages() const {
            return images;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::ShaderConstant>> &getShaderConstants() const {
            return shaderConstants;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Renderable>> &getRenderables() const {
            return renderables;
        }

        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            // I'm not entirely sure what to do for these composite resources; maybe I should make
            // it so that there are CompositeResources as well as just Resources, and then break off the bake()
            // override method so that these composite ones that don't get directly baked will be handled properly lol
            return nullptr;
        }

    private:
        std::vector<std::shared_ptr<scene::Texture>> textures;
        std::vector<std::shared_ptr<scene::Buffer>> buffers;
        std::vector<std::shared_ptr<scene::Image>> images;
        std::vector<std::shared_ptr<scene::ShaderConstant>> shaderConstants;

        // TODO - evaluate whether Materials could be able to have their own renderables? kinda makes sense
        // but i'm not sure if that's really ever used in any material rendering techniques; maybe tho
        std::vector<std::shared_ptr<Renderable>> renderables = {};


    };

}