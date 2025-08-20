#pragma once


#include "../Frontend.hpp"
#include "SceneResources/Material/Material.hpp"
#include "SceneResources/ShaderConstant/ShaderConstant.hpp"
#include "SceneResources/Model/Model.hpp"
#include "SceneResources/Buffer/Buffer.hpp"

#include "ShaderModule/ShaderModule.hpp"

#include "SceneSpace/SceneSpace.hpp"
#include "Camera/Camera.hpp"
#include "SwapchainRenderTarget/SwapchainRenderTarget.hpp"


#include "Light/PointLight.hpp"
#include "RenderGraph/RenderGraph.hpp"

namespace pEngine::girEngine::scene {
    using namespace model;

    namespace graph {
        class RenderGraph;
    }

    class Scene final : public frontend::Frontend {
    public:
        struct CreationInput {
            // optional string label for the scene
            std::string sceneLabel;
            std::string renderGraphLabel;

            bool createDefaultCamera = true;

            bool createSwapchainRenderTargetImage = true;
        };

        explicit Scene(const CreationInput &creationInput);

        ~Scene() override = default;

        void registerModel(const Model &model) {
            models.push_back(model);
        }

        void registerImage(const Image &image) {
            images.push_back(image);
        }

        void registerBuffer(const Buffer &buffer) {
            buffers.push_back(buffer);
        }

        void registerShaderConstant(const ShaderConstant &shaderConstant) {
            shaderConstants.push_back(shaderConstant);
        }

        void registerShaderModule(const ShaderModule &shaderModule) {
            shaderModules.push_back(shaderModule);
        }

        graph::RenderGraph &getRenderGraph() const;

        [[nodiscard]] const std::vector<Model> &getModels() const {
            return models;
        }

        [[nodiscard]] std::vector<ShaderConstant> getShaderConstants() const {
            return shaderConstants;
        }

        //        [[nodiscard]] const std::vector<Material> &getMaterials() const {
        //            return materials;
        //        }

        [[nodiscard]] const std::vector<Image> &getImages() const {
            return images;
        }

        [[nodiscard]] const std::vector<Buffer> &getBuffers() const {
            return buffers;
        }

        //        [[nodiscard]] const std::vector<Texture> &getTextures() const {
        //            return textures;
        //        }

        [[nodiscard]] const std::vector<ShaderModule> &getShaderModules() const {
            return shaderModules;
        }

        const SwapchainRenderTarget &getSwapchainRenderTarget() {
            return swapchainRenderTarget;
        }

        [[nodiscard]] const space::SceneSpace &getSpace() const {
            return space;
        }

        /**
         * This is the function that should trigger all scene objects being converted to
         * GIR, and should return the aggregate list of all the different GIR objects.\n\n
         *
         * @return
         */
        frontend::BakeOutput bakeToGirs() override;

        void registerPointLight(const light::PointLight &pointLight) {
            pointLights.push_back(pointLight);
        }

        Scene(Scene &other)
            : verticalFieldOfView(other.verticalFieldOfView),
              screenWidth(other.screenWidth),
              screenHeight(other.screenHeight),
              label(other.label),
              renderGraph(std::move(other.renderGraph)),
              space(other.space),
              pointLights(other.pointLights),
              shaderModules(other.shaderModules),
              swapchainRenderTarget(other.swapchainRenderTarget),
              images(other.images),
              buffers(other.buffers),
              models(other.models),
              shaderConstants(other.shaderConstants) {
        }


        float verticalFieldOfView;
        int screenWidth;
        int screenHeight;

    private:
        // optional string label for the scene
        std::string label;

        std::unique_ptr<graph::RenderGraph> renderGraph;

        space::SceneSpace space; // TODO -> rip this out/refactor to integrate bullet

        // NEW -> simple point light representation
        std::vector<light::PointLight> pointLights = {};

        std::vector<ShaderModule> shaderModules = {};

        // for the animated model demo I just need access to a render target handle;
        // this probably justifies something a bit fancier/easier to use in the future though
        SwapchainRenderTarget swapchainRenderTarget = SwapchainRenderTarget();

        std::vector<Image> images = {};

        std::vector<Buffer> buffers = {};

        // TODO -> wire textures and materials in

        std::vector<Model> models = {};

        std::vector<ShaderConstant> shaderConstants = {};


        [[nodiscard]] std::vector<gir::SpirVShaderModuleIR>
        bakeShaderModuleGirs() const {
            std::vector<gir::SpirVShaderModuleIR> bakedShaderModuleGIRs = {};

            for (const auto &shaderModule: shaderModules) {
                bakedShaderModuleGIRs.push_back(shaderModule.bakeToGIR());
            }

            return bakedShaderModuleGIRs;
        }

        [[nodiscard]] std::vector<gir::RenderGraphIR>
        bakeRenderGraph(
            const std::vector<gir::BufferIR> &sceneBufferGirs,
            const std::vector<gir::ImageIR> &sceneImageGirs,
            const std::vector<gir::ShaderConstantIR> &sceneShaderConstantGirs,
            const std::vector<gir::model::ModelIR> &modelGirs,
            const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) const;
    };
}
