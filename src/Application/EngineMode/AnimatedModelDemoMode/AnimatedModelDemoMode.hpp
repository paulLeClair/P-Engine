//
// Created by paull on 2024-09-10.
//

#pragma once

#include "../EngineMode.hpp"
#include "../../../../build/minsizerel/_deps/assimp-src/include/assimp/postprocess.h"
#include "../../../GraphicsEngine/Scene/Scene.hpp"

#include "../../../GraphicsEngine/Scene/RenderGraph/RenderGraph.hpp"
#include "../../../GraphicsEngine/Backend/Renderer/PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp"
#include "../../../GraphicsEngine/Backend/Renderer/VulkanRenderer/VulkanRenderer.hpp"
#include "../../../GraphicsEngine/Scene/SceneSpace/Orientation/Orientation.hpp"
#include "../../../GraphicsEngine/Scene/RenderGraph/RenderPass/DynamicRenderPass.hpp"
#include "assimp/Importer.hpp"
#include "../../../utilities/GuiWidget/assimp/AssimpAnimationEditor.hpp"

#include "../../../lib/dear_imgui/imgui.h"

#define MODEL_NAME "Dancing Animated Model from Mixamo"

namespace pEngine::app::mode {
    // these are just meant to match up with the shader's corresponding structs
    struct Light {
        glm::vec4 position = {};
        glm::vec4 color = {};

        Light() = default;

        Light(const glm::vec4 &position, const glm::vec4 &color)
            : position(position),
              color(color) {
        }

        Light(const Light &other) = default;
    };

#define MAX_LIGHTS 10

    struct LightingData {
        glm::vec4 modelUnlitColor = {};
        glm::vec4 warmColor = {};
        glm::vec3 coolColor = {};

        uint32_t lightCount = 0;
        Light lights[10] = {};
    };

    struct CameraData {
        glm::vec3 cameraPosition;
    };

    template<typename GirGeneratorType, typename BackendType>
    class AnimatedModelDemoMode final : public EngineMode<GirGeneratorType, BackendType> {
    public:
        struct CreationInput : EngineMode<GirGeneratorType, BackendType>::CreationInput {
            std::filesystem::path demoModelFileName;
        };

        explicit AnimatedModelDemoMode(const CreationInput &creationInput)
            : EngineMode<GirGeneratorType, BackendType>(creationInput),
              assimpModelPath(creationInput.demoModelFileName) {
            // store a handle to the scene
            scene = std::dynamic_pointer_cast<Scene>(
                this->getEngineCore()->getGraphicsEngine().getScene()
            );

            // store a handle to the backend (this will be written with the vulkan backend; could be changed if there were other backends lol)
            backend = std::dynamic_pointer_cast<backend::vulkan::VulkanBackend>(
                this->getEngineCore()->getGraphicsEngine().getBackend()
            );
        }


        void begin() override {
            // perform all initial scene setup
            prepareScene();

            // twice-bake the scene setup
            auto sceneGirs = scene->bakeToGirs();
            if (backend->bakeGirs(sceneGirs) == backend::GraphicsBackend::BakeResult::FAILURE) {
                // TODO - actual error handling, no throwing
                throw std::runtime_error("Error in AnimatedModelDemoMode::begin() -> unable to bake scene!");
            }

            // show our window via the backend's OS interface
            const auto appContext = std::dynamic_pointer_cast<backend::appContext::vulkan::VulkanApplicationContext>(
                backend->getApplicationContext());

            if (const auto showWindowResult = appContext->getOSInterface()->showWindow();
                showWindowResult != backend::appContext::osInterface::OSInterface::ShowWindowResult::SUCCESS) {
                // TODO -> proper logging!
                return;
            }

            run();
        }

    private:
        // TODO -> these should probably be unique if they aren't needed externally afaik
        std::shared_ptr<Scene> scene;
        std::shared_ptr<backend::vulkan::VulkanBackend> backend;

        std::filesystem::path assimpModelPath;

        glm::vec3 defaultCameraPosition = {0.f, 150.0f, -350.5f};

        space::position::Position defaultModelPosition = space::position::Position({0.0, 0.0f, 0.0f});

        // NEW: we'll have a bunch of gui state here as well
        bool enableFrameByFrame = false;
        bool enableAnimationEditor = false;
        std::vector<glm::mat4> lastPoseCache;
        util::gui::AssimpAnimationEditor::ActiveKeyframeFlag activeKeyframeFlag =
                util::gui::AssimpAnimationEditor::ActiveKeyframeFlag::CURRENT;
        std::unique_ptr<util::gui::AssimpAnimationEditor> animationEditor = nullptr;

        float currentAnimTimepointInSeconds = 0.0f;

        float tickJumpAmount = 5.0f;
        // TODO -> mouse-based camera look-at; for now camera orientation will be hardcoded

        float screenWidth;
        float screenHeight;
        float verticalFieldOfView = glm::radians(40.f);
        float nearPlaneDistance = 0.2f;
        float farPlaneDistance = 10000.f;

        float cameraPitch;
        float cameraYaw;

        Assimp::Importer importer = Assimp::Importer();
        const aiScene *scenePointer;

        std::shared_ptr<Model> model = {};

        // TODO -> move this out of the mode and into the appropriate engine struct; probably the dynamic render pass itself
        static graph::renderPass::VertexInputConfiguration
        obtainVertexInputConfigFromAttributes(const std::vector<geometry::GeometryBinding> &geometryBindings) {
            std::vector<graph::renderPass::VertexInputConfiguration::VertexBindingSlotDescription>
                    inputBindingDescriptions = {};
            unsigned bindingIndex = 0;
            for (auto &geometryBinding: geometryBindings) {
                graph::renderPass::VertexInputConfiguration::VertexBindingSlotDescription description{
                    bindingIndex,
                    geometryBinding.vertexBinding.vertexStride,
                    geometryBinding.vertexBinding.attributes
                };
                inputBindingDescriptions.push_back(description);
                bindingIndex++;
            }
            return graph::renderPass::VertexInputConfiguration{inputBindingDescriptions};
        }

        void prepareScene() {
            initializeSceneVariables();

            loadModelFromAssimpFile();

            // setup animation editor gui widget
            animationEditor = std::make_unique<util::gui::AssimpAnimationEditor>(
                "animation editor",
                const_cast<Model &>(scene->getModels().back()).getAnimation(),
                activeKeyframeFlag
            );

            setupGui();
        }

        void initializeSceneVariables() {
            defaultCameraPosition = {0.f, 150.0f, -650.5f};
            defaultModelPosition = space::position::Position({0.0, 0.0f, 0.0f});

            // TODO -> mouse-based camera look-at; for now camera orientation will be hardcoded

            screenWidth = static_cast<float>(backend->getPresentationEngine().getSwapchain().
                getSwapchainImageWidth());
            screenHeight = static_cast<float>(backend->getPresentationEngine().getSwapchain().
                getSwapchainImageHeight());

            // setup initial static camera variables
            verticalFieldOfView = glm::radians(35.f);
            const auto quat = quat_cast(lookAt(defaultCameraPosition,
                                               defaultModelPosition.position, {0, 1, 0}));
            cameraPitch = pitch(quat);
            cameraYaw = yaw(quat);
            scene->verticalFieldOfView = verticalFieldOfView;
            scene->screenWidth = screenWidth;
            scene->screenHeight = screenHeight;
        }

        void loadModelFromAssimpFile() {
            // define vertex input bindings
            std::vector attributes = {
                geometry::AttributeDescription{
                    "position",
                    // location:
                    0,
                    ResourceFormat::R32G32B32A32_SFLOAT,
                    // offset:
                    0,
                    // label:
                    boost::optional<geometry::AttributeUsage>(
                        geometry::AttributeUsage::POSITION),
                    sizeof(glm::vec4)
                },
                geometry::AttributeDescription{
                    "normal",
                    // location:
                    1,
                    ResourceFormat::R32G32B32A32_SFLOAT,
                    // offset:
                    getFormatElementSizeInBytes(
                        ResourceFormat::R32G32B32A32_SFLOAT),
                    // label:
                    boost::optional<geometry::AttributeUsage>(
                        geometry::AttributeUsage::NORMAL),
                    sizeof(glm::vec4)
                },
                geometry::AttributeDescription{
                    "boneIndices",
                    // location:
                    3,
                    ResourceFormat::R32G32B32A32_UINT,
                    // offset:
                    2 * getFormatElementSizeInBytes(
                        ResourceFormat::R32G32B32A32_SFLOAT),
                    // label:
                    boost::optional<geometry::AttributeUsage>(
                        geometry::AttributeUsage::ANIMATION_BONE_INDICES),
                    sizeof(glm::uvec4)
                },
                geometry::AttributeDescription{
                    "boneWeights",
                    4, // location
                    ResourceFormat::R32G32B32A32_SFLOAT,
                    // offset:
                    // going to be explicit even though these two have the same size
                    2 * getFormatElementSizeInBytes(
                        ResourceFormat::R32G32B32A32_SFLOAT)
                    + getFormatElementSizeInBytes(
                        ResourceFormat::R32G32B32A32_UINT),
                    // label:
                    boost::optional<geometry::AttributeUsage>(
                        geometry::AttributeUsage::ANIMATION_BONE_WEIGHTS),
                    sizeof(glm::vec4)
                }
            };

            unsigned vertexStride = 0u;
            for (auto &attribute: attributes) {
                vertexStride += getFormatElementSizeInBytes(attribute.attributeFormat);
            }
            std::vector vertexInputBindings = {
                geometry::GeometryBinding{
                    0,
                    geometry::VertexDataBinding{
                        // list of attribute descriptions
                        attributes,
                        // vertex stride
                        vertexStride
                    },
                    geometry::IndexDataBinding()
                }
            };

            // build uniform buffer that contains the model's MVP matrix
            auto modelMVPBuffer = Buffer(Buffer::CreationInput{
                "Animated model MVP matrix",
                UniqueIdentifier(),
                Buffer::BufferSubtype::UNIFORM_BUFFER,
                0,
                nullptr,
                sizeof(Model::ModelMatrixBufferData),
                boost::optional<uint32_t>(sizeof(Model::ModelMatrixBufferData))
            });

            // build uniform buffer that contains animated model bone poses for a given frame
            auto bonePosesUniformBuffer = Buffer(Buffer::CreationInput{
                "Animated model demo bone poses buffer",
                UniqueIdentifier(),
                Buffer::BufferSubtype::UNIFORM_BUFFER,
                // may want to add a special subtype to flag that this is for animation bone poses
                1, // TODO -> make the animation bone poses descriptor index a const
                nullptr, // leaving this uninitialized cause the engine will handle updating it;
                sizeof(Model::AnimationBonesUniformBufferData),
                boost::optional<uint32_t>(sizeof(Model::AnimationBonesUniformBufferData)),
                boost::optional<uint32_t>(0),
                boost::optional<bool>(true)
            });

            space::position::Position firstLightPosition({-250.0f, 150.0f, 350.0f});
            auto simplePointLight = light::PointLight{
                .name = "Simple Point Light",
                .identifier = UniqueIdentifier(),
                .position = firstLightPosition,
                .color = glm::vec3(1, 1, 1)
            };
            scene->registerPointLight(simplePointLight);

            space::position::Position secondLightPosition({350.0f, 255.0f, 400.0f});
            auto anotherPointLight = light::PointLight{
                .name = "Another Point Light",
                .identifier = UniqueIdentifier(),
                .position = secondLightPosition,
                .color = glm::vec3(1, 1, 1)
            };

            Light light{
                glm::vec4(simplePointLight.position.getPosition(), 1.0f),
                glm::vec4(simplePointLight.color, 1.0f)
            };
            Light anotherLight{
                glm::vec4(glm::vec3(0.0f, 20.0f, 30.0f), 1.0f),
                glm::vec4(simplePointLight.color /= 2, 1.0f)
            };

            LightingData lightingData = {
                .modelUnlitColor = glm::vec4(0, 0.5, 0, 1.0),
                .warmColor = glm::vec4(0.5, 0.5, 0.0, 1.0),
                .coolColor = glm::vec3(0.1, 0.1, 0.5),
                .lightCount = 2,
                .lights = {light, anotherLight, {}, {}, {}, {}, {}, {}, {}, {}}
            };

            auto lightingDataUniform = Buffer(Buffer::CreationInput{
                .name = "Lighting Data",
                .uid = UniqueIdentifier(),
                .bufferType = Buffer::BufferSubtype::UNIFORM_BUFFER,
                .bindingIndex = 3,
                .initialDataPointer = (unsigned char *) &lightingData,
                .initialDataSizeInBytes = (sizeof(LightingData)),
                .optionalMaxBufferSize = (static_cast<uint32_t>(sizeof(LightingData))),
                .descriptorSetIndex = 0,
                .containsAnimationPoses = false,
            });

            CameraData cameraData = {
                .cameraPosition = defaultCameraPosition
            };

            auto cameraUniform = Buffer(Buffer::CreationInput{
                .name = "Camera Uniform",
                .uid = UniqueIdentifier(),
                .bufferType = Buffer::BufferSubtype::UNIFORM_BUFFER,
                .bindingIndex = 4,
                .initialDataPointer = (unsigned char *) &cameraData,
                .initialDataSizeInBytes = sizeof(CameraData),
                .optionalMaxBufferSize = sizeof(CameraData),
                .descriptorSetIndex = 0,
                .containsAnimationPoses = false,
            });

            // as a simple measure to keep the scene around, we'll move this out now
            importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
            scenePointer = importer.ReadFile(
                assimpModelPath.string(),
                aiProcess_GenSmoothNormals |
                aiProcess_Triangulate |
                aiProcess_OptimizeMeshes |
                aiProcess_FindInvalidData |
                aiProcess_PopulateArmatureData |
                aiProcess_SortByPType |
                aiProcess_JoinIdenticalVertices
            );

            model = std::make_shared<Model>(Model::AssimpCreationInput{
                MODEL_NAME,
                UniqueIdentifier(), // TODO - factor out passing in the UID
                {}, // materials
                {}, // textures
                // NOTE: the design for binding resources will change a lot for the render graph update
                {modelMVPBuffer, bonePosesUniformBuffer, lightingDataUniform, cameraUniform},
                {}, // images
                vertexInputBindings,
                scenePointer,
                defaultModelPosition,
                space::velocity::Velocity(),
                space::orient::Orientation(),
                {1.0, 1.0, 1.0, 1}
            });

            scene->registerModel(*model);

            std::vector models = {*model};
            std::vector drawAttachments = {
                graph::renderPass::DrawAttachment{
                    .models = {*model}
                }

            };

            // render passes will only update resources per-frame that are entered into its dynamic resources map
            std::vector dynamicUniformBuffers = {
                // TODO -> add more facilities for caching results that don't change to reduce memory bandwidth usage
                graph::renderPass::DynamicResourceBinding{
                    .resourceId = bonePosesUniformBuffer.uid,
                    .updateCallback = [&]() -> std::vector<uint8_t> {
                        auto result = std::vector<uint8_t>{};

                        const double currentTimePointInSeconds = enableFrameByFrame
                                                                     ? currentAnimTimepointInSeconds
                                                                     :
#if WIN32
                                                                     GetTickCount64() / 1000.0;
#endif

                        // NEW: externalized assimp animation data which samples the aiScene's animation directly
                        if (scene->getModels().empty())
                            return {};
                        if (scene->getModels().size() > 1) {
                            // TODO -> log a warning here!
                        }
                        auto &currentModel = const_cast<Model &>(scene->getModels().back());

                        auto &animation = currentModel.getAnimation();
                        std::vector<glm::mat4> animationPoses;
                        if (enableAnimationEditor) {
                            // use the editor's channels
                            animationPoses = animation.obtainAnimationPoses(
                                currentTimePointInSeconds, animationEditor->currentAnimationChannels);
                        } else {
                            // use the un-edited animation channels generally;
                            animationPoses = animation.obtainAnimationPoses(
                                currentTimePointInSeconds, animation.animationChannels);
                        }
                        if (animationPoses.empty()) {
                            // TODO -> log a warning here!
                        }

                        if (enableFrameByFrame && animation.cacheDebugInfo) {
                            switch (activeKeyframeFlag) {
                                case util::gui::AssimpAnimationEditor::ActiveKeyframeFlag::PREVIOUS: {
                                    animationPoses = animation.cache.previousKeyframe.finalizedBonePoses;
                                    break;
                                }
                                case util::gui::AssimpAnimationEditor::ActiveKeyframeFlag::NEXT: {
                                    animationPoses = animation.cache.nextKeyframe.finalizedBonePoses;
                                    break;
                                }
                                default:
                                    break;
                            }
                        }

                        result.resize(animationPoses.size() * sizeof(glm::mat4));
                        std::memcpy(result.data(), animationPoses.data(), result.size());

                        return result;
                    },
                    .priority = 3.0f
                },
                graph::renderPass::DynamicResourceBinding{
                    .resourceId = cameraUniform.uid,
                    .updateCallback = [&]() -> std::vector<uint8_t> {
                        auto result = std::vector<uint8_t>{};
                        result.resize(sizeof(cameraData.cameraPosition));
                        std::memcpy(result.data(), &cameraData.cameraPosition, sizeof(cameraData.cameraPosition));

                        return result;
                    },
                    .priority = 1.0f
                },
                graph::renderPass::DynamicResourceBinding{
                    .resourceId = modelMVPBuffer.uid,
                    .updateCallback = [&]() -> std::vector<uint8_t> {
                        auto result = std::vector<uint8_t>{};

                        auto modelMVPMatrix = glm::mat4(1.0f);

                        auto handednessConversionMatrix = glm::mat4(1.0);
                        handednessConversionMatrix[1][1] = -1;

                        modelMVPMatrix *= view::Camera::computePerspectiveTransform(
                            scene->verticalFieldOfView,
                            (float) scene->screenWidth / (float) scene->screenHeight,
                            nearPlaneDistance,
                            farPlaneDistance
                        );

                        modelMVPMatrix *= handednessConversionMatrix;

                        modelMVPMatrix *= view::Camera::computeViewTransform(
                            defaultCameraPosition,
                            cameraYaw,
                            cameraPitch
                        );

                        auto modelItr = std::ranges::find_if(scene->getModels(), [&](const Model &sceneModel) {
                            return sceneModel.name == MODEL_NAME;
                        });
                        if (modelItr == scene->getModels().end()) {
                            return {};
                        }
                        modelMVPMatrix *= modelItr->getModelMatrix();

                        result.resize(2 * sizeof(glm::mat4));
                        std::memcpy(result.data(), &modelMVPMatrix, sizeof(glm::mat4));
                        // TODO -> formalize normals matrix computation; for now I'm just gonna have them be the same
                        std::memcpy(result.data() + sizeof(glm::mat4), &modelMVPMatrix, sizeof(glm::mat4));

                        return result;
                    },
                    .priority = 2.0f
                },
                graph::renderPass::DynamicResourceBinding{
                    .resourceId = lightingDataUniform.uid,
                    .updateCallback = [&]() -> std::vector<uint8_t> {
                        auto result = std::vector<uint8_t>{};

                        static LightingData data = {
                            .modelUnlitColor = glm::vec4(0, 0.5, 0, 1.0),
                            .warmColor = glm::vec4(0.5, 0.5, 0.0, 1.0),
                            .coolColor = glm::vec3(0.1, 0.1, 0.5),
                            .lightCount = 2,
                            .lights = {
                                {
                                    glm::vec4(glm::vec3{-250.0f, 150.0f, 350.0f}, 1.0f),
                                    glm::vec4(glm::vec3(1, 1, 1), 1.0f)
                                },
                                {
                                    glm::vec4(glm::vec3{350.0f, 255.0f, 400.0f}, 1.0f),
                                    glm::vec4(glm::vec3(1, 1, 1) /= 2, 1.0f)
                                },
                                {}, {}, {}, {}, {}, {}, {}, {}
                            }
                        };

                        result.resize(sizeof(LightingData));
                        std::memcpy(result.data(), &data, sizeof(LightingData));

                        return result;
                    },
                    .priority = 4.0f
                },

            };


            auto modelRenderPass = std::make_shared<graph::renderPass::DynamicRenderPass>();
            setupModelRenderPass(vertexInputBindings, dynamicUniformBuffers, modelRenderPass);

            scene->getRenderGraph().addDynamicRenderPass(*modelRenderPass);
        }

        void setupModelRenderPass(const std::vector<geometry::GeometryBinding> &vertexInputBindings,
                                  const std::vector<graph::renderPass::DynamicResourceBinding> &dynamicUniformBuffers,
                                  const std::shared_ptr<graph::renderPass::DynamicRenderPass> &modelRenderPass) const {
            // provide basic model render pass info
            modelRenderPass->name = "Model Pass";
            modelRenderPass->uid = UniqueIdentifier();

            // setup graphics pipeline for pass
            modelRenderPass->graphicsPipelineConfiguration = graph::renderPass::GraphicsPipelineConfiguration{
                {}, //default color blend
                // TODO -> un-hardcode color blend matching with ImGui and instead plug it in here
                {
                    .enableDepthTesting = true,
                    .enableDepthWrites = true,
                    .depthTestingOperation
                    = graph::renderPass::DepthStencilConfiguration::CompareOperation::LESS,
                },
                {}, //default dynamic state
                {}, //default multisample config
                {
                    // primitive assembly config:
                    false,
                    {
                        graph::renderPass::PrimitiveAssemblyConfiguration::PrimitiveTopology::TRIANGLE_LIST
                    }
                },
                {}, // default rasterization config(?)
                {}, // default tessellation
                obtainVertexInputConfigFromAttributes(vertexInputBindings),
            };

            // setup vertex input binding descriptions for model meshes
            modelRenderPass->geometryBindings = vertexInputBindings;

            // setup dynamic resources (just uniforms for now)
            modelRenderPass->dynamicBuffers = dynamicUniformBuffers;

            // register our assimp-loaded model for drawing
            modelRenderPass->bindModelsForDrawing(0,
                                                  {*model});

            // TODO -> do we need to be wrapping these in smart pointers anymore?
            const auto vertexShaderModule = std::make_shared<ShaderModule>(ShaderModule::CreationInput{
                "Animated Model Demo Vertex Shader Module",
                UniqueIdentifier(),
                "DemoVertexShader",
                "main",
                ShaderModule::ShaderUsage::VERTEX_SHADER,
                ShaderModule::ShaderLanguage::GLSL
            });

            const auto fragmentShaderModule =
                    std::make_shared<ShaderModule>(ShaderModule::CreationInput{
                        "Animated Model Demo Fragment Shader Module",
                        UniqueIdentifier(),
                        "DemoFragmentShader",
                        "main",
                        ShaderModule::ShaderUsage::FRAGMENT_SHADER,
                        ShaderModule::ShaderLanguage::GLSL
                    });

            // set up vertex and fragment shaders and
            scene->registerShaderModule(*vertexShaderModule);
            scene->registerShaderModule(*fragmentShaderModule);
            modelRenderPass->vertexShaderAttachment = graph::renderPass::ShaderAttachment{
                vertexShaderModule.get()
            };
            modelRenderPass->fragmentShaderAttachment = graph::renderPass::ShaderAttachment{
                fragmentShaderModule.get()
            };

            // attach swapchain image render target
            modelRenderPass->colorAttachments.push_back(graph::renderPass::ImageAttachment{
                scene->getSwapchainRenderTarget().getSwapchainImage().uniqueIdentifier,
                // MAKE SURE the format is actually set!
                scene->getSwapchainRenderTarget().getSwapchainImage().getFormat(),
                graph::renderPass::AttachmentState::SWAPCHAIN_COLOR_ATTACHMENT,
            });

            // add depth attachment
            const auto depthImage = std::make_shared<Image>(
                Image::CreationInput{
                    .name = "Depth Image",
                    .uid = UniqueIdentifier(),
                    .format = ResourceFormat::D32_SFLOAT,
                    .imageUsages = Image::ImageUsage::DepthStencilAttachment,
                    .width = backend->getPresentationEngine().getSwapchain().getSwapchainImageWidth(),
                    .height = backend->getPresentationEngine().getSwapchain().getSwapchainImageHeight(),
                    .initialImageData = nullptr,
                    .initialImageDataSizeInBytes = 0,
                    // TODO -> factor out this redundant extent (which can be obtained from width&height above)
                    .imageExtent = Image::ImageExtent2D{
                        backend->getPresentationEngine().getSwapchain().getSwapchainImageWidth(),
                        backend->getPresentationEngine().getSwapchain().getSwapchainImageHeight()
                    }
                }
            );
            scene->registerImage(*depthImage);

            // enable depth test stuff
            modelRenderPass->depthAttachment = graph::renderPass::ImageAttachment{
                depthImage->uniqueIdentifier,
                depthImage->getFormat(),
                graph::renderPass::AttachmentState::DEPTH_ATTACHMENT,
                {},
                graph::renderPass::MultisampleResolveOperation::NO_OP,
                graph::renderPass::AttachmentState::UNDEFINED,
                graph::renderPass::AttachmentLoadOperation::CLEAR,
                graph::renderPass::AttachmentStoreOperation::DONT_CARE
            };
            modelRenderPass->depthTestEnabled = true;
            modelRenderPass->depthWriteEnabled = true;
        }

        void setupGui() {
            // b. set up gui callbacks
            tickJumpAmount = 5.0f;

            auto callbacks = [&]() {
                const ImGuiIO &io = ImGui::GetIO();
                (void) io;

                ImGui::Begin("Model Viewer Demo");

                ImGui::Text("Part of the pre-pre-alpha of the GirEngine game engine framework!");
                ImGui::NewLine();
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

                ImGui::Separator();
                ImGui::Checkbox("Enable Frame-by-Frame Mode", &enableFrameByFrame);

                if (scene->getModels().empty())
                    return;
                if (scene->getModels().size() > 1) {
                    // TODO -> log a warning here!
                }
                auto &currentModel = const_cast<Model &>(scene->getModels().back());
                assimp::SkeletalAnimation &animation = currentModel.getAnimation();
                const auto *anim = animation.animation;
                if (anim == nullptr) {
                    // TODO -> log!
                    return;
                }
                const double ticks_per_second = anim->mTicksPerSecond;
                const auto animDurationInSeconds = static_cast<float>(
                    anim->mDuration / ticks_per_second);

                ImGui::SliderFloat("Current Frame Time", &currentAnimTimepointInSeconds, 0.0f,
                                   animDurationInSeconds,
                                   "%.3f s");
                if (ImGui::Button("Forward")) {
                    currentAnimTimepointInSeconds += tickJumpAmount / ticks_per_second;
                    if (currentAnimTimepointInSeconds > animDurationInSeconds) currentAnimTimepointInSeconds = 0.0f;
                }
                ImGui::SameLine();
                if (ImGui::Button("Backward")) {
                    currentAnimTimepointInSeconds -= tickJumpAmount / ticks_per_second;
                    if (currentAnimTimepointInSeconds < 0.0f) currentAnimTimepointInSeconds = animDurationInSeconds;
                }
                ImGui::InputFloat("Number of ticks to jump by", &tickJumpAmount);

                // Display current frame info
                ImGui::Text("Tick: %.2f / %.2f", currentAnimTimepointInSeconds * ticks_per_second,
                            animDurationInSeconds * ticks_per_second);
                ImGui::Checkbox("Enable Animation Editor", &enableAnimationEditor);
                // animation.cacheDebugInfo = enableKeyframeViewer;
                if (animation.cacheDebugInfo) animationEditor->updateCache(std::move(animation.cache));
                if (enableAnimationEditor) {
                    animationEditor->toCallback()();
                }

                ImGui::End();
            };
            backend->getRenderer().setGuiCallbacks({callbacks});
        }

        void run() const {
            bool keepRendering = true;
            while (keepRendering) {
#ifdef _WIN32
                // TODO -> factor out and make this win32 loop structure more robust
                // (so it is convenient and can be reused across modes)
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

                const auto drawFrameResult = backend->drawFrame();
                keepRendering = drawFrameResult == backend::GraphicsBackend::DrawFrameResult::SUCCESS;
            }
        }
    };
}
