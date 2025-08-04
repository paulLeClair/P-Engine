#pragma once

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../Camera/Camera.hpp"
#include "../../GraphicsIR/RenderGraphIR/RenderGraphIR.hpp"
#include "RenderPass/DynamicRenderPass.hpp"

namespace pEngine::girEngine::scene {
    class Scene;
}

#include <vector>
#include <string>

namespace pEngine::girEngine::scene::graph {
    class RenderGraph {
    public:
        struct CreationInput {
            std::string name;
            UniqueIdentifier uid;

            Scene &scene;
        };

        ~RenderGraph() = default;

        explicit RenderGraph(const CreationInput &creationInput)
            : name(creationInput.name), uid(creationInput.uid), parentScene(creationInput.scene) {
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const UniqueIdentifier &getUid() const {
            return uid;
        }

        /**
         * This should add a render pass object to the graph for execution; render passes should be
         * executed in the order they're added
        */
        void addDynamicRenderPass(const renderPass::DynamicRenderPass &pass);

        /**
         * Get list of render passes in the order they were added
        */
        [[nodiscard]] std::vector<renderPass::DynamicRenderPass> &getDynamicRenderPasses() {
            return dynamicRenderPasses;
        }

        /**
         * Ultra-simplified "bake" process where really the render graph does not exist and is pre-linearized;
         * a future update with a full render graph implementation is on the way.
         * @return
         */
        gir::RenderGraphIR bakeRenderGraphToGIR(
            const std::vector<gir::BufferIR> &sceneBufferGirs,
            const std::vector<gir::ImageIR> &sceneImageGirs,
            const std::vector<gir::ShaderConstantIR> &sceneShaderConstantGirs,
            const std::vector<gir::model::ModelIR> &modelGirs,
            const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs
        ) {
            return gir::RenderGraphIR(
                gir::RenderGraphIR::CreationInput{
                    name,
                    uid,
                    gir::GIRSubtype::RENDER_GRAPH,
                    bakeSceneDynamicRenderPassesToGIR(
                        sceneBufferGirs,
                        sceneImageGirs,
                        sceneShaderConstantGirs,
                        modelGirs,
                        sceneShaderModuleGirs)
                });
        }

    private:
        std::string name;
        UniqueIdentifier uid;

        // for the animated model demo, the render graph is assumed to be pre-linearized (until render graph rewrite)
        std::vector<renderPass::DynamicRenderPass> dynamicRenderPasses = {};

        Scene &parentScene;

        std::vector<DynamicRenderPassIR>
        bakeSceneDynamicRenderPassesToGIR(
            const std::vector<gir::BufferIR> &sceneBufferGirs,
            const std::vector<gir::ImageIR> &sceneImageGirs,
            const std::vector<gir::ShaderConstantIR> &sceneShaderConstantGirs,
            const std::vector<gir::model::ModelIR> &modelGirs,
            const std::vector<gir::SpirVShaderModuleIR> &sceneShaderModuleGirs) const {
            std::vector<DynamicRenderPassIR> renderPassGIRs = {};

            // traverse render passes and bake them in order
            for (auto &renderPass: dynamicRenderPasses) {
                renderPassGIRs.push_back(
                    renderPass.bakeToGIR(sceneBufferGirs,
                                         sceneImageGirs,
                                         sceneShaderConstantGirs,
                                         modelGirs,
                                         sceneShaderModuleGirs)
                );
            }

            return renderPassGIRs;
        }
    };
}
