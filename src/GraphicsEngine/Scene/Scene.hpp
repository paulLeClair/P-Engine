#pragma once

#include "../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include "SceneResources/SceneResource.hpp"
#include "SceneResources/Texture/Texture.hpp"
#include "SceneResources/Material/Material.hpp"
#include "SceneResources/Renderable/Renderable.hpp"
#include "SceneResources/ShaderConstant/ShaderConstant.hpp"
#include "SceneResources/Model/Model.hpp"
#include "SceneResources/Buffer/Buffer.hpp"

#include "ShaderModule/ShaderModule.hpp"

#include "RenderGraph/RenderGraph.hpp"
#include "../GraphicsIR/GirGenerator.hpp"

#include <unordered_set>

namespace pEngine::girEngine::gir {
    // forward declarations for graphics IR classes
    class GraphicsIntermediateRepresentation;
}

namespace pEngine::girEngine::scene {


    /**
     * Now that we're trying to get the core menu engine mode running, I have to reconsider the
     * previous decision I made to re-combine the implementation & interface for the scene.
     *
     * This is because we need to be able to specify the scene type as a template argument
     * for the stuff that's provided for the application/engine mode etc stuff.
     *
     * I think an easy workaround is to just define a GIRGenerator base class? Then the scene
     * can inherit only the bakeToGir stuff from that class, and we can have it be abstract
     * so it's not a huge change. Then you just change the template argument to not be
     * GirGeneratorType but instead GirGeneratorType. Seems to make sense!
     */
    class Scene : public gir::generator::GirGenerator {
    public:
        struct CreationInput {
            // optional string label for the scene
            std::string sceneLabel;
            std::string renderGraphLabel;
        };

        explicit Scene(const CreationInput &creationInput)
                : renderGraph(std::make_unique<scene::graph::RenderGraph>(scene::graph::RenderGraph::CreationInput{
                creationInput.renderGraphLabel,
                util::UniqueIdentifier()
        })) {

        }

        ~Scene() = default;

        void registerModel(const std::shared_ptr<Model> &model) {
            models.push_back(model);
        }

        void registerImage(const std::shared_ptr<Image> &image) {
            images.push_back(image);
        }

        void registerBuffer(const std::shared_ptr<Buffer> &buffer) {
            buffers.push_back(buffer);
        }

        void registerShaderConstant(const std::shared_ptr<ShaderConstant> &shaderConstant) {
            shaderConstants.push_back(shaderConstant);
        }

        void registerShaderModule(const std::shared_ptr<ShaderModule> &shaderModule) {
            shaderModules.push_back(shaderModule);
        }

        scene::graph::RenderGraph &getSceneRenderGraph() {
            return *renderGraph;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Model>> &getModels() const {
            return models;
        }

        [[nodiscard]] std::vector<std::shared_ptr<ShaderConstant>> getShaderConstants() const {
            return shaderConstants;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Material>> &getMaterials() const {
            return materials;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Image>> &getImages() const {
            return images;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Buffer>> &getBuffers() const {
            return buffers;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<Texture>> &getTextures() const {
            return textures;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<ShaderModule>> &getShaderModules() const {
            return shaderModules;
        }

        [[maybe_unused]][[nodiscard]] std::vector<std::shared_ptr<Resource>> getAllResources() const {
            std::vector<std::shared_ptr<Resource>> allResources = {};

            for (auto &buffer: buffers) {
                allResources.push_back(buffer);
            }

            for (auto &image: images) {
                allResources.push_back(image);
            }

            for (auto &texture: textures) {
                allResources.push_back(texture);
            }

            for (auto &material: materials) {
                allResources.push_back(material);
            }

            for (auto &renderable: renderables) {
                for (auto &renderableVertexBuffer: renderable->getVertexBuffers()) {
                    allResources.push_back(renderableVertexBuffer);
                }
                for (auto &renderableIndexBuffer: renderable->getIndexBuffers()) {
                    allResources.push_back(renderableIndexBuffer);
                }
            }

            for (auto &model: models) {
                allResources.push_back(model);
            }

            for (auto &shaderConstant: shaderConstants) {
                allResources.push_back(shaderConstant);
            }

            return allResources;
        }


        /**
         * This is the function that should trigger all scene objects being converted to
         * GIR, and should return the aggregate list of all the different GIR objects.\n\n
         *
         * @return
         */
        [[nodiscard]] std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>>
        bakeToGirs() const override {
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> resourceGirs
                    = bakeSceneResourceGirs();

            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> shaderModuleGirs
                    = bakeShaderModuleGirs();

            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> renderGraphGirs
                    = bakeRenderGraph(resourceGirs, shaderModuleGirs);

            return concatenateBakedGirs(
                    resourceGirs,
                    shaderModuleGirs,
                    renderGraphGirs);
        }


    private:
        // optional string label for the scene
        std::string label;

        std::unique_ptr<scene::graph::RenderGraph> renderGraph;

        std::vector<std::shared_ptr<scene::ShaderModule>> shaderModules = {};

        std::vector<std::shared_ptr<scene::Image>> images = {};

        std::vector<std::shared_ptr<scene::Buffer>> buffers = {};

        std::vector<std::shared_ptr<scene::Texture>> textures = {};

        std::vector<std::shared_ptr<scene::Material>> materials = {};

        std::vector<std::shared_ptr<scene::Renderable>> renderables = {};

        std::vector<std::shared_ptr<scene::Model>> models = {};

        std::vector<std::shared_ptr<scene::ShaderConstant>> shaderConstants = {};

        [[nodiscard]] std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>>
        bakeSceneResourceGirs() const {
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> bakedResourceGIRs = {};

            for (const auto &sceneResource: getAllResources()) {
                bakedResourceGIRs.push_back(sceneResource->bakeToGIR());
            }

            return bakedResourceGIRs;
        }

        [[nodiscard]] std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>>
        bakeShaderModuleGirs() const {
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> bakedShaderModuleGIRs = {};

            for (const auto &shaderModule: shaderModules) {
                bakedShaderModuleGIRs.push_back(shaderModule->bakeToGIR());
            }

            return bakedShaderModuleGIRs;
        }

        [[nodiscard]] std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>>
        bakeRenderGraph(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &bakedSceneResourceGirs,
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &bakedShaderModuleGirs) const {
            return renderGraph->bakeRenderGraphToGIR(bakedSceneResourceGirs, bakedShaderModuleGirs);
        }

        static std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>>
        concatenateBakedGirs(const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &resourceGirs,
                             const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &shaderModuleGirs,
                             const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &renderGraphGirs) {
            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> bakedSceneGIRList = {};

            bakedSceneGIRList.insert(bakedSceneGIRList.end(), resourceGirs.begin(), resourceGirs.end());
            bakedSceneGIRList.insert(bakedSceneGIRList.end(), shaderModuleGirs.begin(), shaderModuleGirs.end());
            bakedSceneGIRList.insert(bakedSceneGIRList.end(), renderGraphGirs.begin(), renderGraphGirs.end());

            if (bakedSceneGIRList.size() != resourceGirs.size() + shaderModuleGirs.size() + renderGraphGirs.size()) {
                throw std::runtime_error("Error in Scene::concatenateBakedGirs() -> result not equal to the sum "
                                         "of lengths of the lists being concatenated!");
            }

            return bakedSceneGIRList;
        }

    };

}