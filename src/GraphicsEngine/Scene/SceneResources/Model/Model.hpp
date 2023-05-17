#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "../../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include "../SceneResource.hpp"
#include "../Renderable/Renderable.hpp"
#include "../Material/Material.hpp"

using namespace PUtilities;

class Model : public SceneResource {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;
        const std::string &name;
        PUtilities::UniqueIdentifier uid;

        std::vector<std::shared_ptr<Renderable>> renderables;
        std::vector<std::shared_ptr<Material>> materials;
        std::vector<std::shared_ptr<Texture>> textures;
        std::vector<std::shared_ptr<Buffer>> buffers;
        std::vector<std::shared_ptr<Image>> images;
        std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;
    };

    explicit Model(const CreationInput &creationInput) {

    }

    ~Model() override = default;

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Renderable>> &getRenderables() const {
        return renderables;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<Material>> &getMaterials() const {
        return materials;
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

    UpdateResult update() override {
        return UpdateResult::FAILURE;
    }

private:
    std::shared_ptr<Scene> parentScene;

    std::string name;
    PUtilities::UniqueIdentifier uniqueIdentifier;

    std::vector<std::shared_ptr<Renderable>> renderables;
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<std::shared_ptr<Buffer>> buffers;
    std::vector<std::shared_ptr<Image>> images;
    std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;

};
