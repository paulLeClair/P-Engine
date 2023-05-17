//
// Created by paull on 2022-06-17.
//

#include "PScene.hpp"

namespace PGraphics {

    std::shared_ptr<Model> PScene::getModel(const std::string &name) const {
        if (name.empty() || modelNamesToIndices.count(name) != 1) {
            return nullptr;
        }

        return models[modelNamesToIndices.at(name)];
    }

    std::shared_ptr<Image> PScene::getImage(const std::string &name) const {
        if (name.empty() || imageNamesToIndices.count(name) != 1) {
            return nullptr;
        }

        return images[imageNamesToIndices.at(name)];
    }

    std::shared_ptr<Buffer> PScene::getBuffer(const std::string &name) const {
        if (name.empty() || bufferNamesToIndices.count(name) != 1) {
            return nullptr;
        }

        return buffers[bufferNamesToIndices.at(name)];
    }

    std::shared_ptr<ShaderModule> PScene::getShaderModule(const std::string &name) const {
        if (name.empty() || shaderModuleNamesToIndices.count(name) != 1) {
            return nullptr;
        }

        return shaderModules[shaderModuleNamesToIndices.at(name)];
    }

    const std::vector<std::shared_ptr<Model>> &PScene::getModels() const {
        return models;
    }

    std::vector<std::shared_ptr<SceneResource>> PScene::getAllResources() const {
        std::vector<std::shared_ptr<SceneResource>> aggregatedResources = {};
        for (const auto &image: images) {
            aggregatedResources.push_back(std::dynamic_pointer_cast<SceneResource>(image));
        }

        for (const auto &buffer: buffers) {
            aggregatedResources.push_back(std::dynamic_pointer_cast<SceneResource>(buffer));
        }

        return aggregatedResources;
    }

    const std::vector<std::shared_ptr<ShaderModule>> &PScene::getShaderModules() const {
        return shaderModules;
    }

    const std::vector<std::shared_ptr<Material>> &PScene::getMaterials() const {
        return materials;
    }

    const std::vector<std::shared_ptr<Texture>> &PScene::getTextures() const {
        return textures;
    }

    std::vector<std::shared_ptr<ShaderConstant>> PScene::getShaderConstants() const {
        return shaderConstants;
    }

}// namespace PGraphics