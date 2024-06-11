#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include "../SceneResource.hpp"
#include "../Renderable/Renderable.hpp"
#include "../Material/Material.hpp"
#include "../Buffer/Buffer.hpp"

namespace pEngine::girEngine::scene {

    /**
     * This class will likely need redesigning soon - I'm not entirely sure about certain things here.
     *
     * I think the main point is that Models should store Renderables (which have to maintain geometry data)
     * or it can be handed vertex & index buffers directly.
     *
     * Maybe to match the new GIR "DrawType" thing in the DrawAttachmentIR class we can
     * have something like that here.
     */
    class Model : public scene::Resource {
    public:
        struct CreationInput : public scene::Resource::CreationInput {
            std::vector<std::shared_ptr<Renderable>> renderables;
            std::vector<std::shared_ptr<Material>> materials;
            std::vector<std::shared_ptr<Texture>> textures;
            std::vector<std::shared_ptr<Buffer>> buffers;
            std::vector<std::shared_ptr<Image>> images;
            std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;
        };

        explicit Model(const CreationInput &creationInput) :
                Resource(creationInput),
                renderables(creationInput.renderables),
                materials(creationInput.materials),
                textures(creationInput.textures),
                buffers(creationInput.buffers),
                images(creationInput.images),
                shaderConstants(creationInput.shaderConstants) {

        }

        ~Model() override = default;

        [[nodiscard]] const std::vector<std::shared_ptr<Renderable>> &getRenderables() const {
            return renderables;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Material>> &getMaterials() const {
            return materials;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &getTextures() const {
            return textures;
        }

        // TODO - evaluate not splitting up buffers by usage, like here (and rely on buffers storing their own usage)
        [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getBuffers() const {
            return buffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Image>> &getImages() const {
            return images;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ShaderConstant>> &getShaderConstants() const {
            return shaderConstants;
        }

        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        /**
         * Since models are aggregate resources, I'm not entirely sure what should happen here...
         * I think because we're assuming the model's sub-resources already exist when the model
         * is created, we can basically return nothing here...?
         *
         * Similarly to what I was thinking for scene::Materials, I think we'll eventually break up
         * the scene::Resource classes into scene::CompositeResource and scene::Resource
         *
         * @return nullptr, since models themselves just aggregate other resources
         */
        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return nullptr;
        }

    private:
        std::shared_ptr<Scene> parentScene;

        std::string name;
        util::UniqueIdentifier uniqueIdentifier;

        std::vector<std::shared_ptr<Renderable>> renderables;
        std::vector<std::shared_ptr<Material>> materials;
        std::vector<std::shared_ptr<Texture>> textures;
        std::vector<std::shared_ptr<Buffer>> buffers;
        std::vector<std::shared_ptr<Image>> images;
        std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;

    };

}