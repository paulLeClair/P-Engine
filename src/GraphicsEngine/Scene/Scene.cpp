//
// Created by paull on 2025-04-24.
//
#include "Scene.hpp"

#include "RenderGraph/RenderGraph.hpp"

namespace pEngine::girEngine::scene {
    Scene::Scene(const CreationInput &creationInput)
        : swapchainRenderTarget(SwapchainRenderTarget::CreationInput{
            boost::optional<std::string>(creationInput.sceneLabel + " Swapchain Render Target Image Handle"),
            boost::none, //
            boost::optional<glm::vec3>({0.4, 0.4, 0.4})
        }) {
        renderGraph = std::make_unique<graph::RenderGraph>(graph::RenderGraph::CreationInput{
            creationInput.renderGraphLabel,
            UniqueIdentifier(),
            *this
        });

        if (creationInput.createSwapchainRenderTargetImage) {
            images.push_back(swapchainRenderTarget.getSwapchainImage());
        }

        space = std::make_unique<space::SceneSpace>(space::SceneSpace::CreationInput{
            space::SpaceVolumeOptions::INFINITE_VOLUME
        });

        if (creationInput.createDefaultCamera) {
            // here we'll just create a simple default camera
            auto defaultCameraUid = UniqueIdentifier();
            auto defaultCamera = std::make_shared<view::Camera>(view::Camera::CreationInput{
                "defaultCamera",
                defaultCameraUid,
                space->requestPositionHandle(defaultCameraUid),
                space->requestVelocityHandle(defaultCameraUid)
            });
            // default: camera will be at rest at the origin of worldspace (which should be kinda the centerpoint?)

            activeCameraIndex = 0;
        }
    }

    gir::generator::GirGenerator::BakeOutput Scene::bakeToGirs() {
        std::vector<gir::BufferIR> bufferGirs = {};
        for (auto &buffer: buffers) {
            bufferGirs.push_back(buffer.bakeBuffer());
        }
        std::vector<gir::ImageIR> imageGirs = {};
        for (auto &image: images) {
            // the swapchain image abstraction will probably change, but for now you just add it as a regular image gir
            imageGirs.push_back(image.bakeImage());
        }

        std::vector<gir::ShaderConstantIR> shaderConstantGirs = {}; // not used currently

        std::vector<gir::model::ModelIR> modelGirs = {};
        for (graph::renderPass::DynamicRenderPass &renderPass: renderGraph->getDynamicRenderPasses()) {
            // TODO -> generalize this to all buffers after single-animated-model demo
            for (auto &buffer: renderPass.uniformBuffers) {
                bufferGirs.push_back(buffer.bakeBuffer());
            }
            for (graph::renderPass::DrawAttachment &drawBinding: renderPass.drawBindings) {
                for (Model &model: drawBinding.models) {
                    for (auto &buffer: model.getBuffers()) {
                        bufferGirs.push_back(buffer.bakeBuffer());
                    }

                    modelGirs.push_back(model.bakeModel());
                }
            }
        }

        const std::vector<gir::SpirVShaderModuleIR> shaderModuleGirs
                = bakeShaderModuleGirs();

        return BakeOutput{
            bufferGirs, // buffers
            imageGirs, // images
            shaderConstantGirs, // shader constants (still todo)
            modelGirs, // models
            {}, // model-less draw attachments (not used currently)
            bakeCameras(), // cameras
            bakeLights(),
            shaderModuleGirs, // shaders
            bakeRenderGraph(bufferGirs,
                            imageGirs,
                            shaderConstantGirs,
                            modelGirs,
                            shaderModuleGirs) // render graphs
        };
    }

    std::vector<gir::RenderGraphIR> Scene::bakeRenderGraph(
        const std::vector<gir::BufferIR> &sceneBufferGirs,
        const std::vector<gir::ImageIR> &sceneImageGirs,
        const std::vector<gir::ShaderConstantIR> &sceneShaderConstantGirs,
        const std::vector<gir::model::ModelIR> &modelGirs,
        const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) const {
        return {
            renderGraph->bakeRenderGraphToGIR(sceneBufferGirs,
                                              sceneImageGirs,
                                              sceneShaderConstantGirs,
                                              modelGirs,
                                              sceneShaderModuleGirs)
        };
    }

    graph::RenderGraph &Scene::getRenderGraph() const {
        return *renderGraph;
    }
}
