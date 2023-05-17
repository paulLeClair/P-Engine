//
// Created by paull on 2022-06-17.
//

#pragma once

#include <memory>
#include <unordered_set>

#include "../Scene.hpp"
#include "../SceneRenderGraph/PSceneRenderGraph/PSceneRenderGraph.hpp"
#include "../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../ShaderModule/PShaderModule/PShaderModule.hpp"
#include "../SceneResources/Buffer/Buffer.hpp"
#include "../SceneResources/Model/Model.hpp"
#include "../SceneResources/Image/Image.hpp"

namespace PGraphics {

    class PScene : public Scene {
    public:
        struct CreationInput {
            // TODO - scene configuration as needed
        };

        explicit PScene(const CreationInput &creationInput) {
            renderGraph = std::make_shared<PSceneRenderGraph>();
        }

        ~PScene() = default;

        void registerModel(const std::shared_ptr<Model> &model) override {
            models.push_back(model);
            modelNamesToIndices[model->getName()] = models.size() - 1;
        }

        [[nodiscard]] std::shared_ptr<Model> getModel(const std::string &name) const override;

        [[nodiscard]] std::shared_ptr<Model>
        getModel(const PUtilities::UniqueIdentifier &uniqueIdentifier) const override {
            return models[modelUIDsToIndices.at(uniqueIdentifier)];
        }

        void registerImage(const std::shared_ptr<Image> &image) override {
            images.push_back(image);
            imageNamesToIndices[image->getName()] = images.size() - 1;
        }

        [[nodiscard]] std::shared_ptr<Image> getImage(const std::string &name) const override;

        [[nodiscard]] std::shared_ptr<Image>
        getImage(const PUtilities::UniqueIdentifier &uniqueIdentifier) const override {
            if (imageUIDsToIndices.count(uniqueIdentifier) == 0) {
                // TODO - throw an exception here probably!
            }

            return images[imageUIDsToIndices.at(uniqueIdentifier)];
        }

        void registerBuffer(const std::shared_ptr<Buffer> &buffer) override {

            buffers.push_back(buffer);
            bufferNamesToIndices[buffer->getName()] = buffers.size() - 1;
        }

        [[nodiscard]] std::shared_ptr<Buffer> getBuffer(const std::string &name) const override;

        [[nodiscard]] std::shared_ptr<Buffer>
        getBuffer(const PUtilities::UniqueIdentifier &uniqueIdentifier) const override {
            return buffers[bufferUIDsToIndices.at(uniqueIdentifier)];
        }

        void
        registerShaderModule(const std::shared_ptr<ShaderModule> &shaderModule) override {

            shaderModules.push_back(shaderModule);
            shaderModuleNamesToIndices[shaderModule->getName()] = shaderModules.size() - 1;
        }

        std::shared_ptr<SceneRenderGraph> getSceneRenderGraph() override {
            return renderGraph;
        }

        void registerShaderConstant(const std::shared_ptr<ShaderConstant> &shaderConstant) override {
            shaderConstants.push_back(shaderConstant);
            shaderConstantNamesToIndices[shaderConstant->getName()] = shaderConstants.size() - 1;
            shaderConstantUIDsToIndices[shaderConstant->getUniqueIdentifier()] = shaderConstants.size() - 1;
        }

        [[nodiscard]] std::shared_ptr<ShaderModule> getShaderModule(const std::string &name) const override;

        [[nodiscard]] const std::vector<std::shared_ptr<Model>> &getModels() const override;

        [[nodiscard]] const std::vector<std::shared_ptr<Image>> &getImages() const override {
            return images;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getBuffers() const override {
            return buffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Material>> &getMaterials() const override;

        [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &getTextures() const override;

        [[nodiscard]] const std::vector<std::shared_ptr<ShaderModule>> &getShaderModules() const override;

        [[nodiscard]] std::vector<std::shared_ptr<SceneResource>> getAllResources() const override;

        // TODO - split this off into a separate class (same with Scene::PerFrameUpdates) and just have them provide
        // an interface that gives sets of updated models/images/buffers as we have here (but without having to directly modify members of a struct
        // which is not very functional)
        struct PerFrameUpdates : public Scene::PerFrameUpdates {
            std::unordered_set<PUtilities::UniqueIdentifier> updatedModels = {};
            std::unordered_set<PUtilities::UniqueIdentifier> updatedImages = {};
            std::unordered_set<PUtilities::UniqueIdentifier> updatedBuffers = {};
            std::unordered_set<PUtilities::UniqueIdentifier> updatedTextures = {};
        };

        // TODO - if necessary add in return type enums if we want to be able to hand back more information about failures etc
        void addModelToPerFrameUpdates(const PUtilities::UniqueIdentifier &uniqueIdentifier) {
            perFrameUpdates.updatedModels.insert(uniqueIdentifier);
        }

        // TODO - evaluate whether we need to give the option to access by name string
        void addModelToPerFrameUpdates(const std::string &name) {
            auto updatedModelUid = models[modelNamesToIndices[name]]->getUniqueIdentifier();
            addModelToPerFrameUpdates(updatedModelUid);
        }

        void addImageToPerFrameUpdates(const PUtilities::UniqueIdentifier &uniqueIdentifier) {
            perFrameUpdates.updatedImages.insert(uniqueIdentifier);
        }

        void addImageToPerFrameUpdates(const std::string &name) {
            auto updatedImageUid = images[imageNamesToIndices[name]]->getUniqueIdentifier();
            addImageToPerFrameUpdates(updatedImageUid);
        }

        void addBufferToPerFrameUpdates(const PUtilities::UniqueIdentifier &uniqueIdentifier) {
            perFrameUpdates.updatedBuffers.insert(uniqueIdentifier);
        }

        void addBufferToPerFrameUpdates(const std::string &name) {
            auto updatedBufferUid = buffers[bufferNamesToIndices[name]]->getUniqueIdentifier();
            addBufferToPerFrameUpdates(updatedBufferUid);
        }

        void addTextureToPerFrameUpdates(const std::string &name) {
            auto updatedTextureUid = textures[textureNamesToIndices[name]]->getUniqueIdentifier();
            addTextureToPerFrameUpdates(updatedTextureUid);
        }

        void addTextureToPerFrameUpdates(const PUtilities::UniqueIdentifier &uniqueIdentifier) {
            perFrameUpdates.updatedTextures.insert(uniqueIdentifier);
        }

        [[nodiscard]] const Scene::PerFrameUpdates &getPerFrameUpdates() const override {
            // TODO - add some sanity checking here, plus logging if there are any things that seem insane
            return (const Scene::PerFrameUpdates &) perFrameUpdates;
        }

        [[nodiscard]] std::vector<std::shared_ptr<ShaderConstant>> getShaderConstants() const override;

    private:
        std::shared_ptr<SceneRenderGraph> renderGraph;

        // TODO - factor out a "SceneResourceRegistry" type class that takes a scene resource template argument
        // (should be able to replace all this boilerplate)
        // TODO also - consider whether that should just be a utilities class since it's such a common thing I'm doing
        std::vector<std::shared_ptr<ShaderModule>> shaderModules;
        std::unordered_map<std::string, unsigned long> shaderModuleNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> shaderModuleUIDsToIndices;

        std::vector<std::shared_ptr<Image>> images;
        std::unordered_map<std::string, unsigned long> imageNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> imageUIDsToIndices;

        std::vector<std::shared_ptr<Buffer>> buffers;
        std::unordered_map<std::string, unsigned long> bufferNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> bufferUIDsToIndices;

        std::vector<std::shared_ptr<Texture>> textures;
        std::unordered_map<std::string, unsigned long> textureNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> textureUIDsToIndices;

        std::vector<std::shared_ptr<Material>> materials;
        std::unordered_map<std::string, unsigned long> materialNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> materialUIDsToIndices;

        std::vector<std::shared_ptr<Renderable>> renderables;
        std::unordered_map<std::string, unsigned long> renderableNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> renderableUIDsToIndices;

        std::vector<std::shared_ptr<Model>> models;
        std::unordered_map<std::string, unsigned long> modelNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> modelUIDsToIndices;

        std::vector<std::shared_ptr<ShaderConstant>> shaderConstants;
        std::unordered_map<std::string, unsigned long> shaderConstantNamesToIndices;
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> shaderConstantUIDsToIndices;


        /**
         * New idea: all objects inside a scene store a ref to the parent scene (slight memory sacrifice but it's nothing).
         * Then, somehow we have to hand off these updated values...
         *
         * Maybe we just hand them off in a struct, and assume that it won't be called until after all application-side update
         * code has finished (onus on the user)
         *
         * That would then require that each update
         */
        PerFrameUpdates perFrameUpdates;

    };
}// namespace PGraphics
