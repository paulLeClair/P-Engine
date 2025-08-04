#pragma once


#include "SceneResources/Texture/Texture.hpp"
#include "SceneResources/Material/Material.hpp"
#include "SceneResources/ShaderConstant/ShaderConstant.hpp"
#include "SceneResources/Model/Model.hpp"
#include "SceneResources/Buffer/Buffer.hpp"

#include "ShaderModule/ShaderModule.hpp"

#include "../GraphicsIR/GirGenerator.hpp"
#include "SceneSpace/SceneSpace.hpp"
#include "Camera/Camera.hpp"
#include "SwapchainRenderTarget/SwapchainRenderTarget.hpp"

#include "../GraphicsIR/light/PointLightIR/PointLightIR.hpp"

#include "Light/PointLight.hpp"

namespace pEngine::girEngine::scene {
    using namespace model;

    namespace graph {
        class RenderGraph;
    }

    class Scene final : public gir::generator::GirGenerator {
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

        void registerCamera(const view::Camera &camera) {
            cameras.push_back(camera);
        }

        bool setActiveCamera(int newCameraIndex) {
            if (newCameraIndex < 0 || newCameraIndex > cameras.size() - 1) {
                return false;
            }

            activeCameraIndex = newCameraIndex;
            return true;
        }

        [[nodiscard]] int getCurrentCameraIndex() const {
            return activeCameraIndex;
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
            return *space;
        }

        [[nodiscard]] std::vector<gir::camera::CameraGIR> bakeCameras() {
            std::vector<gir::camera::CameraGIR> cameraGirs = {};

            for (view::Camera &camera: cameras) {
                cameraGirs.push_back(
                    camera.bakeToGIR()
                );
            }

            return cameraGirs;
        }

        /**
         * This is the function that should trigger all scene objects being converted to
         * GIR, and should return the aggregate list of all the different GIR objects.\n\n
         *
         * @return
         */
        BakeOutput bakeToGirs() override;

        void registerPointLight(const light::PointLight &pointLight) {
            pointLights.push_back(pointLight);
        }

        float verticalFieldOfView;
        int screenWidth;
        int screenHeight;

    private:
        // optional string label for the scene
        std::string label;

        std::unique_ptr<graph::RenderGraph> renderGraph;

        std::unique_ptr<space::SceneSpace> space; // TODO -> rip this out/refactor to integrate bullet

        // NEW -> simple point light representation
        std::vector<light::PointLight> pointLights = {};

        std::vector<view::Camera> cameras = {};
        int activeCameraIndex = -1;

        std::vector<ShaderModule> shaderModules = {};

        // for the animated model demo I just need access to a render target handle;
        // this probably justifies something a bit fancier/easier to use in the future though
        SwapchainRenderTarget swapchainRenderTarget = SwapchainRenderTarget();

        std::vector<Image> images = {};

        std::vector<Buffer> buffers = {};

        // TODO -> wire textures and materials in

        std::vector<Model> models = {};

        std::vector<ShaderConstant> shaderConstants = {};


        /**
         * Ultra basic light bake code ->
         */
        std::vector<gir::light::PointLightIR> bakeLights() {
            std::vector<gir::light::PointLightIR> girs = {};
            for (auto &light: pointLights) {
                girs.emplace_back(
                    light.name,
                    light.identifier,
                    light.position.getPosition(),
                    light.color
                );
            }
            return girs;
        }


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
