//
// Created by paull on 2022-09-02.
//

#pragma once

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>

#include "../EngineMode.hpp"
#include "../../../GraphicsEngine/Backend/ApplicationContext/ApplicationContext.hpp"
#include "../../../GraphicsEngine/Backend/Renderer/Renderer.hpp"
#include "../../../GraphicsEngine/Scene/RenderGraph/RenderPass/ImguiRenderPass/ImguiRenderPass.hpp"
#include "../../../GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"

using namespace pEngine::core;

// BACK TO THIS SHITE AGAIN!
// now that the backend can draw frames, we can probably try to get this core menu
// mode running;

namespace pEngine::app::mode {
    template<typename GirGeneratorType, typename BackendType>
    class CoreMenuEngineMode : public EngineMode<GirGeneratorType, BackendType> {
    public:
        struct CreationInput : public EngineMode<GirGeneratorType, BackendType>::CreationInput {
            std::vector<std::function<void()> > initialDearImguiGuiCallbacks = {};
            bool shouldBypassMainLoop = false;
        };

        explicit CoreMenuEngineMode(const CreationInput &creationInput)
                : EngineMode<GirGeneratorType, BackendType>(creationInput),
                  shouldBypassMainLoop(creationInput.shouldBypassMainLoop),
                  dearImguiGuiCallbacks(creationInput.initialDearImguiGuiCallbacks) {
            // store a handle to the scene
            scene = std::dynamic_pointer_cast<scene::Scene>(
                    this->getEngineCore()->getGraphicsEngine().getScene()
            );

            // store a handle to the backend (this will be written with the vulkan backend; could be changed if there were other backends lol)
            backend = std::dynamic_pointer_cast<backend::vulkan::VulkanBackend>(
                    this->getEngineCore()->getGraphicsEngine().getBackend()
            );
        }

        ~CoreMenuEngineMode() = default;

        /**
         *
         * @return
         */
        [[nodiscard]] const std::vector<std::function<void()> > &getGuiElementCallbacks() const {
            return dearImguiGuiCallbacks;
        }

        void begin() override {
            // TODO - reorganize this; it's kinda clunky
            setUpScene();
            bake();
            run();
        }

    private:
        static const std::string DEFAULT_IMGUI_RENDER_PASS_NAME;

        bool shouldBypassMainLoop;

        std::shared_ptr<scene::Scene> scene;

        std::shared_ptr<backend::vulkan::VulkanBackend> backend;

        std::vector<std::function<void()> > dearImguiGuiCallbacks = {};

        enum class PrepareSceneResult {
            FAILURE,
            SUCCESS
        };

        std::vector<std::function<void()>> obtainDearImguiCoreMenuCallbacks() {
            // I guess for the demo I'll start with a simple gui window that's customized to show off some shtuff
            static float f = 0.0f;
            static int counter = 0;

            static bool show_demo_window = false;
            std::vector<float> &clear_color = backend->getRenderer()->acquireClearColorHandle();

            /**
             * For now, this will just be a big ugly monolithic callback, but a polished project
             * would probably break everything down and would allow for more flexible usage of the GUI library.
             */
            const auto &exampleWindow = [&]() {
                ImGuiIO &io = ImGui::GetIO();
                (void) io;

                ImGui::Begin("Core Menu Demo");

                ImGui::Text("This is the pre-pre-alpha of the GirEngine game engine framework!");
                ImGui::NewLine();
                ImGui::Text(
                        "Specifically, this is demoing the most basic GirEngine application, which consists of a single DearImgui render pass "
                        "being drawn using FIFO presentation mode.");
                ImGui::NewLine();
                ImGui::Text(
                        "I'm trying to design the engine in a similar way to a compiler, with a front-end, IR, and back-end.");

                ImGui::Text("Current features:");

                /**
                 * SCENE DESCRIPTION
                 */
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0}, "-girEngine::");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.2, 0.8, 0.8, 1.0}, "Scene");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0},
                                   "-> render graph abstraction and principal interface with user's application code");

                ImGui::Indent();

                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- designed to act as a ");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.9, 0.2, 0.9, 1.0}, "backend-agnostic");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0}, " front-end to different graphics backends");

                ImGui::TextColored({0.6, 0.6, 0.9, 1.0},
                                   "- outputs a set of GraphicsIntermediateRepresentation objects (described below) for actual rendering of the specified scene");

                ImGui::TextColored({0.6, 0.6, 0.6, 1.0}, "- create render passes and define dependencies between them");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- provide shader binaries directly, easily bind them to render passes");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- shader resource binding information can be reflected, which simplifies scene resource bindings in the render graph");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- define image, buffer, and texture resources to be bound to render passes");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- high-level graphics pipeline configuration designed with automation in mind");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- partial implementation of user-specified geometry specification (full implementation upcoming)");

                ImGui::Unindent();

                /**
                 * GIR DESCRIPTION
                 */
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0}, "-girEngine::");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.9, 0.3, 0.8, 1.0}, "GraphicsIntermediateRepresentation");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0},
                                   "-> simple intermediate layer between front-end and back-end");
                ImGui::Indent();
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- packages data supplied by the front-end into simple structs that provide context about how the information should be used in graphics operations");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- intended to be processed by a girEngine backend to execute the encoded graphics operations");
                ImGui::Unindent();

                /**
                 * BACKEND DESCRIPTION
                 */
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0}, "-girEngine::");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.4, 0.4, 0.8, 1.0}, "VulkanBackend");
                ImGui::SameLine(0.0, 0.0);
                ImGui::TextColored({0.1, 0.6, 0.2, 1.0}, "-> Foundational Vulkan backend implementation");
                ImGui::Indent();
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- prepares required data and facilitates real-time rendering using the Vulkan 1.3 API");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- operates by accepting a set of GraphicsIntermediateRepresentation objects as input");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- translates backend-agnostic info into the analogous Vulkan data structures & API calls");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- integration of SPIRVReflect for SPIR-V shader module reflection");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- aiming to automate a lot of boilerplate code, but with configurability where needed");
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- provides general interface for integration with different OS window systems");
                ImGui::Unindent();

                ImGui::TextColored({0.1, 0.6, 0.2, 1.0},
                                   "- Basic integration with Windows, partial integration with Linux");
                ImGui::Indent();
                ImGui::TextColored({0.6, 0.6, 0.6, 1.0},
                                   "- pre-alpha window system integration and input handling");
                ImGui::Unindent();

                ImGui::NewLine();
                ImGui::ColorEdit3("clear color", (float *) clear_color.data());
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

                ImGui::Text("Some DearImGui demo features:");
                ImGui::Checkbox("Additional Demo Window",
                                &show_demo_window);

                ImGui::End();

                if (show_demo_window) {
                    ImGui::Begin("Additional demo window!");
                    ImGui::Text("This should just demonstrate a few other simple Imgui doodads.");

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

                    if (ImGui::Button(
                            "Increment Counter")) {
                        counter++;
                    }
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);
                    if (ImGui::Button("Reset Counter")) {
                        counter = 0;
                    }

                    // Generate samples and plot them
                    static float samples[100];
                    for (int n = 0; n < 100; n++)
                        samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
                    ImGui::PlotLines("Sine Wave plot", samples, 100);

                    // Display contents in a scrolling region
                    ImGui::TextColored(ImVec4(1, 0.5, 0.5, 1), "Big Bordered List of Items");

                    ImGui::BeginChild("Demo-Window-Scrolling", ImVec2(0, 0), true);
                    for (int n = 0; n < 50; n++)
                        ImGui::Text("List Item number: %04d", n + 1);
                    ImGui::EndChild();

                    ImGui::End();
                }
            };

            return {exampleWindow};
        }

        PrepareSceneResult prepareScene() {
            if (scene == nullptr) {
                // TODO - better logging/error codes/use of error codes/whatever else i should be doing lol
                return PrepareSceneResult::FAILURE;
            }
            // for now, maybe we can just create a single imgui render pass?
            // we'll use no resources, and only the simple vertex and fragment shaders

            // one thing i have to figure out now: going back to some of the windowing/presentation stuff
            // and also revisiting the scene's abstraction for render targets...
            // the ideal behavior would be if it just generates as many render targets as your
            // chosen presentation mode (which I'll also have to add to the scene I think)
            // tbh, that stuff might all go into the scene bake code anyway; by default it can generate
            // a set of render targets or else the user can specify all of them. But it will force the user
            // to match up the number of manually-specified render targets to be at least as many is required
            // for the presentation mode chosen.
            // i can't remember the specifics of all that shit in Vulkan so I'll have to do some reviewing as well.

            // create basic unused vertex shader (for now?)
            const std::shared_ptr<scene::ShaderModule> &vertexShaderModule = std::make_shared<scene::ShaderModule>(
                    scene::ShaderModule::CreationInput{
                            "coreMenuUnusedBasicVertexShader",
                            util::UniqueIdentifier(),
                            "testVertShader", // filename used for test vertex shader included ATOW
                            "main",
                            scene::ShaderModule::ShaderUsage::VERTEX_SHADER,
                            scene::ShaderModule::ShaderLanguage::GLSL
                    });
            scene->registerShaderModule(vertexShaderModule);

            // create basic unused fragment shader (for now?)
            const std::shared_ptr<scene::ShaderModule> &fragmentShaderModule = std::make_shared<scene::ShaderModule>(
                    scene::ShaderModule::CreationInput{
                            "coreMenuUnusedBasicFragmentShader",
                            util::UniqueIdentifier(),
                            "testFragShader", // filename used for test fragment shader included ATOW
                            "main",
                            scene::ShaderModule::ShaderUsage::FRAGMENT_SHADER,
                            scene::ShaderModule::ShaderLanguage::GLSL
                    });
            scene->registerShaderModule(fragmentShaderModule);

            std::shared_ptr<scene::graph::renderPass::ImguiRenderPass> dearImguiRenderPass = std::make_shared<
                    scene::graph::renderPass::ImguiRenderPass>(
                    scene::graph::renderPass::ImguiRenderPass::CreationInput{
                            "coreMenuRenderPass",
                            util::UniqueIdentifier(),
                            scene::graph::renderPass::RenderPass::Subtype::DEAR_IMGUI_RENDER_PASS,
                            obtainGraphicsPipelineConfiguration(),
                            obtainDearImguiCoreMenuCallbacks()
                    });
            scene->getSceneRenderGraph().addNewRenderPass(dearImguiRenderPass);

            return PrepareSceneResult::SUCCESS;
        }

        void showApplicationWindow() {
            // TODO
        }

        void setUpScene() {
            if (prepareScene() != PrepareSceneResult::SUCCESS) {
                // TODO - better logging and error handling
                throw std::runtime_error("Error in CoreMenuEngineMode::setUpScene() - scene preparation failed!");
            }
        }

        void bake() {
            // TODO - maybe add some way to sanity-check the scene here? can be a future issue once the basic stuff works

            // first: bake scene
            const auto sceneGirs = scene->bakeToGirs();

            // second: bake backend using scene bake output
            if (backend->bakeGirs(sceneGirs) == backend::GraphicsBackend::BakeResult::FAILURE) {
                throw std::runtime_error("Error in CoreMenuEngineMode::bake(): backend bake failed!");
            }

            // TODO - probably add something in somewhere to ensure that the output of the backend bake makes sense
        }

        void run() {
            if (shouldBypassMainLoop) {
                return;
            }

            // show window I guess?
            auto appContext = std::dynamic_pointer_cast<backend::appContext::vulkan::VulkanApplicationContext>(
                    backend->getApplicationContext());
            auto showWindowResult = appContext->getOSInterface()->showWindow();

            if (showWindowResult != backend::appContext::osInterface::OSInterface::ShowWindowResult::SUCCESS) {
                std::cout << "Show window failed! Aborting run()..." << std::endl;
                return;
            }

            // TODO - write some kind of update->render loop!
            bool keepRendering = true;
            while (keepRendering) {
                // TODO - any scene updating stuff would go here (nothing for core menu demo tho; this will come later)

#ifdef _WIN32
                // WIN32 requires you to get messages in this loop
                MSG uMsg;
                while (PeekMessage(
                        &uMsg,
                        0, //NOLINT
                        0,
                        0,
                        PM_REMOVE
                )) {
                    TranslateMessage(&uMsg);
                    DispatchMessage(&uMsg);
                }
#endif

                auto drawFrameResult = backend->drawFrame();
                keepRendering = drawFrameResult == backend::GraphicsBackend::DrawFrameResult::SUCCESS;
            }

            // use default behavior to finish engine mode execution
            finish();
        }

        void finish() {
            // TODO - better logging (using an actual logging library)
            std::cout << "Core Menu Mode has completed." << std::endl;
        }


        scene::graph::renderPass::pipeline::GraphicsPipelineConfiguration obtainGraphicsPipelineConfiguration() {
            // I'm actually pretty sure we can just use the defaults... specify particular configurations if needed
            // (on the other hand we might end up adding to this so it might change anyway - i'll leave it explicit 4 now)
            return scene::graph::renderPass::pipeline::GraphicsPipelineConfiguration{
                    // color blend
                    {}, // use default
                    // depth/stencil
                    {}, // use default
                    // dynamic state
                    {}, // use default
                    // multisample
                    {}, // use default
                    // primitive assembly
                    {}, // use default
                    // rasterization
                    {}, // use default
                    // tessellation
                    {}, // use default
                    // vertex input
                    {} // use default
            };
        }

    };
} // namespace PEngine
