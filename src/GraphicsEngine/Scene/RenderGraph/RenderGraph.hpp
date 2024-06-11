#pragma once

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../SceneView/SceneView.hpp"
#include "RenderPass/RenderPass.hpp"
#include "../../GraphicsIR/RenderGraphIR/RenderGraphIR.hpp"

#include <vector>
#include <string>

namespace pEngine::girEngine::scene::graph {

    class RenderGraph {
    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;
        };

        ~RenderGraph() = default;

        explicit RenderGraph(const CreationInput &creationInput)
                : name(creationInput.name), uid(creationInput.uid) {

        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const UniqueIdentifier &getUid() const {
            return uid;
        }

        [[nodiscard]] const std::vector<std::shared_ptr<View>> &getSceneViews() const {
            return sceneViews;
        }

        [[nodiscard]] int getActiveSceneViewIndex() const {
            return activeSceneViewIndex;
        }

        /**
         * Returning to this as part of:
         * https://github.com/jabronicus/p-engine-develop/issues/93
         *
         * For now, this interface will be as minimal and simplistic as possible, I'll extend it in the
         * future if needed
         *
         * Review:
         * - scene itself is the repository for all parts of the scene
         * - each render graph (the scene may as well be able to support more than one?) is
         *   responsible for defining a sequence of render passes and the inputs/outputs that they use
         *
         * Now that I'm actually trying to implement this, I'm not super sure if we want to tie in any
         * other parts of the scene (eg viewpoints) - I imagine we'll probably have to.
         *
         * I think we can have each render pass maintain its inputs and outputs (although maybe not we'll see)
         * and then we can maintain a viewpoint etc
         *
         * I think for the most part we can just use the render graph to store the render passes and their order,
         * as well as a view point for each render pass. That should hopefully make it a bit easier if we wanted
         * to do some kind of wacky multiple-viewpoint stuff (but that's probably not super practical lol)
         */


        /**
         * This should add a render pass object to the graph for execution; render passes should be
         * executed in the order they're added
        */
        void addNewRenderPass(const std::shared_ptr<RenderPass> &pass) {
            renderPasses.push_back(pass);
        }

        /**
         * Get list of render passes in the order they were added
        */
        [[nodiscard]] const std::vector<std::shared_ptr<RenderPass>> &getRenderPasses() const {
            return renderPasses;
        }

        /**
         * The scene view stuff is all stub for now - we can add it in probably last,
         * since it's kind of self-contained and will only be relevant once we are ready
         * to actually draw and present frames.
         *
         * @param view
         */
        void addSceneView(const std::shared_ptr<scene::View> &view) {
            sceneViews.push_back(view);
        }

        /**
         * This should set the active viewpoint by UID; this would be useful if you wanted
         * to put together a cutscene or something like that, or generally if you want to maintain multiple
         * viewpoints.\n\n
         *
         * Note that the view stuff is TODO until we get to the point where we need a camera into the scene
         *
         * Not sure how useful a feature that is but it might make certain things easier, and regardless
         * you have to set up a camera to even have a scene
         *
         * @param viewUid
         */
        void setActiveSceneView(const util::UniqueIdentifier &viewUid) {
            int index = 0;
            for (auto &view: sceneViews) {
                if (view->getUid() == viewUid) {
                    activeSceneViewIndex = index;
                    return;
                }
                index++;
            }

            // TODO - log!
        }

        [[nodiscard]] const std::shared_ptr<View> &getActiveSceneView() const {
            if (activeSceneViewIndex < 0) {
                throw std::runtime_error("Error in RenderGraph::getActiveSceneView() - "
                                         "Active scene index acquired before being set!");
            }
            return sceneViews[activeSceneViewIndex];
        }

        /**
         * I think if we just commit to working with batches of gir objects that the Backend will have to
         * split up itself, that should do for now. We can always refactor and reorganize what happens here. \n\n
         *
         * @param sceneResourceGIRs - vector of already-baked resources for the scene (all resources must be baked first!)
         * @param shaderModuleGIRs - vector of already-baked shader modules for the scene (all shader modules must be baked first!)
         * @return -> The vector of GIR objects that were created from the render graph, **not including the resources and shader modules**
         */
        std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> bakeRenderGraphToGIR(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneResourceGIRs,
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &shaderModuleGIRs
        ) {
            // okay so here's where we use the pre-baked resources & shader modules to assemble our render passes

            // at this point, i'm starting to see where it might be useful to create an encapsulating GIR class
            // that is basically a struct of arrays (of structs lol) to delineate the usages of each gir class.

            // on the other hand, everytime I think about doing that, I remember if we just use the fact that
            // each GIR object can identify its own renderPassSubtype, then the backend can do any struct-of-array assembling
            // that it wants to do, and we can keep it as simple as possible...

            // I guess for the most part it'll just spit out render pass IR, but I think the easiest
            // way to do that would be to instead create another composite IR class (probably "RenderGraphIR")
            // which would batch up the render pass GIRs (and maybe serve some other purpose down the line)

            // In that case, we really only have to bake the render graph IR class for now (scene views and
            // anything else that ends up being relevant to render graphs will come later)

            std::shared_ptr<gir::RenderGraphIR> renderGraphIR = std::make_shared<gir::RenderGraphIR>(
                    gir::RenderGraphIR::CreationInput{
                            name,
                            uid,
                            gir::GIRSubtype::RENDER_GRAPH,
                            bakeSceneRenderPassesToGIR(sceneResourceGIRs, shaderModuleGIRs)
                    });

            std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> renderGraphGIRs = {renderGraphIR};

            // TODO - add views and any other GIR objects that need to be created (eventually move graphics pipelines here too)

            return renderGraphGIRs;
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        std::vector<std::shared_ptr<RenderPass>> renderPasses = {};

        std::vector<std::shared_ptr<View>> sceneViews = {};
        int activeSceneViewIndex = -1;

        std::vector<std::shared_ptr<RenderPassIR>>
        bakeSceneRenderPassesToGIR(
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneResourceGirs,
                const std::vector<std::shared_ptr<gir::GraphicsIntermediateRepresentation>> &sceneShaderModuleGirs) {
            std::vector<std::shared_ptr<RenderPassIR>> renderPassGIRs = {};

            // traverse render passes and bake them in order
            for (auto &renderPass: renderPasses) {
                renderPassGIRs.push_back(
                        std::dynamic_pointer_cast<RenderPassIR>(
                                renderPass->bakeToGIR(sceneResourceGirs, sceneShaderModuleGirs)
                        )
                );
            }

            return renderPassGIRs;
        }
    };
}