//
// Created by paull on 2022-07-17.
//

#pragma once

#include <memory>
#include <vector>

#include "../Buffer/Buffer.hpp"
#include "../Texture/Texture.hpp"

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
            std::vector<std::shared_ptr<scene::Buffer> > initialBuffers;
            std::vector<std::shared_ptr<scene::Image> > initialImages;
            std::vector<std::shared_ptr<scene::Texture> > initialTextures;
        };

        explicit Material(const CreationInput &creationInput)
            : Resource(creationInput),
              textures(creationInput.initialTextures),
              buffers(creationInput.initialBuffers),
              images(creationInput.initialImages) {
        }

        Material(): Resource({}) {
        }

        ~Material() override = default;

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Texture> > &getTextures() const {
            return textures;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Buffer> > &getBuffers() const {
            return buffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<scene::Image> > &getImages() const {
            return images;
        }

        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return nullptr;
        }

    private:
        std::vector<std::shared_ptr<scene::Texture> > textures;
        std::vector<std::shared_ptr<scene::Buffer> > buffers;
        std::vector<std::shared_ptr<scene::Image> > images;
    };
}
