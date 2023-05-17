//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>
#include <utility>

#include "../../../EngineCore/ThreadPool/ThreadPool.hpp"
#include "../ApplicationContext/PApplicationContext/PApplicationContext.hpp"
#include "../Backend.hpp"
#include "../../../EngineCore/ThreadPool/PThreadPool/PThreadPool.hpp"
#include "../FrameExecutionController/PFrameExecutionController/PFrameExecutionController.hpp"
#include "../../Scene/PScene/PScene.hpp"
#include "../../Scene/SceneResources/Buffer/Buffer.hpp"
#include "../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.hpp"
#include "VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp"
#include "VulkanTexture/VulkanTexture.hpp"
#include "VulkanPushConstant/VulkanPushConstant.hpp"
#include "VulkanRenderable/VulkanRenderable.hpp"
#include "../../Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"
#include "../../Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.hpp"

namespace PGraphics {

    static const int DEFAULT_NUMBER_OF_BUFFERED_FRAMES = 3;

    static const int DEFAULT_NUMBER_OF_DESCRIPTOR_SET_SLOTS = 4;

    class PBackend : public Backend {
    public:
        friend class PGraphicsEngine;

        struct CreationInput {
            std::shared_ptr<PEngine::PThreadPool> threadPool = nullptr;

            std::shared_ptr<PScene> scene = nullptr;
        };

        explicit PBackend(const CreationInput &createInfo) : threadPool(createInfo.threadPool),
                                                             scene(createInfo.scene) {
            applicationContext = std::make_shared<PApplicationContext>();

            initializeVmaAllocator();

            std::vector<VkDescriptorSetLayout> descriptorSetLayouts(DEFAULT_NUMBER_OF_DESCRIPTOR_SET_SLOTS);

            for (int descriptorSetSlotIndex = 0;
                 descriptorSetSlotIndex < DEFAULT_NUMBER_OF_DESCRIPTOR_SET_SLOTS; descriptorSetSlotIndex++) {
                VulkanDescriptorSetAllocator::CreationInput descriptorSetAllocatorCreationInput{
                        static_cast<unsigned int>(descriptorSetSlotIndex),
                        applicationContext->getLogicalDevice(),
                        descriptorSetLayouts[descriptorSetSlotIndex],
                        VulkanDescriptorSetAllocator::DEFAULT_DESCRIPTOR_POOL_SIZES_VECTOR
                };

                descriptorSetAllocators[descriptorSetSlotIndex] = std::make_shared<VulkanDescriptorSetAllocator>(
                        descriptorSetAllocatorCreationInput);
            }

            initializeFrameExecutionController(createInfo);
        }

        void initializeVmaAllocator() {
            allocator = VK_NULL_HANDLE;

            VmaAllocatorCreateInfo allocatorCreateInfo = {
                    0,
                    applicationContext->getPhysicalDevice(),
                    applicationContext->getLogicalDevice(),
                    0,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    applicationContext->getInstance(),
                    0};

            if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create VmaAllocator!");
            }
        }

        void initializeFrameExecutionController(const CreationInput &createInfo) {
            PFrameExecutionController::CreationInput frameContextCreateInfo = {applicationContext,
                                                                               createInfo.scene,
                                                                               threadPool,
                                                                               DEFAULT_NUMBER_OF_BUFFERED_FRAMES};
            frameExecutionController = std::make_shared<PFrameExecutionController>(frameContextCreateInfo);
        }

        ~PBackend() = default;

        DrawFrameResult drawFrame() override;

        std::shared_ptr<ApplicationContext> getApplicationContext() override {
            return std::dynamic_pointer_cast<ApplicationContext>(applicationContext);
        }

        std::shared_ptr<FrameExecutionController> getFrameExecutionController() override {
            return std::dynamic_pointer_cast<FrameExecutionController>(frameExecutionController);
        }

        void
        bakeRenderData() override; // protected so it can be used as a friend function - TODO: evaluate whether to use this design (seems ugly to me)

    private:
        bool hasBeenBaked = false;

        VmaAllocator allocator;

        std::shared_ptr<PScene> scene;

        std::shared_ptr<PEngine::PThreadPool> threadPool;

        std::shared_ptr<PApplicationContext> applicationContext;

        // TODO -> break up the Backend class into a namespace and have the various parts be free-floating;
        // need to figure out a nice design but also simpler is better so idk what direction to take exactly
        std::shared_ptr<PFrameExecutionController> frameExecutionController;


        std::shared_ptr<VulkanBufferSuballocator> uniformBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> vertexIndexBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> texelBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> storageBufferSuballocator;

        std::map<unsigned int, std::shared_ptr<VulkanDescriptorSetAllocator>> descriptorSetAllocators;

        std::vector<std::shared_ptr<VulkanImage>> vulkanImages = {};
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> vulkanImageUids = {};

        std::vector<std::shared_ptr<VulkanTexture>> vulkanTextures = {};
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> vulkanTextureUids = {};

        std::vector<std::shared_ptr<VulkanPushConstant>> vulkanPushConstants = {};
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> vulkanPushConstantUids = {};

        std::vector<std::shared_ptr<VulkanShaderModule>> vulkanShaderModules = {};
        std::unordered_map<PUtilities::UniqueIdentifier, unsigned long> vulkanShaderModuleUids = {};

        std::vector<std::shared_ptr<VulkanRenderPass>> vulkanRenderPasses = {};
        std::shared_ptr<DearImguiVulkanRenderPass> dearImguiRenderPass = nullptr; // TODO - allow user to configure where in the render graph the built-in GUI pass will go (eg before, after, or custom)

        std::unordered_set<PUtilities::UniqueIdentifier> vulkanRenderableUids = {};

        // TODO - add any other Vulkan objects that have to be baked

        enum class CommitPerFrameUpdatesResult {
            SUCCESS,
            FAILURE
        };

        CommitPerFrameUpdatesResult commitPerFrameUpdates(const PScene::PerFrameUpdates &perFrameUpdates);

        [[nodiscard]] bool commitBufferUpdate(const PUtilities::UniqueIdentifier &bufferUid) const;

        [[nodiscard]] bool commitImageUpdate(const PUtilities::UniqueIdentifier &imageUid) const;

        [[nodiscard]] bool commitModelUpdate(const PUtilities::UniqueIdentifier &modelUid) const;

        /**
         * TODO - break off all these private bake*() functions into their own SUBCLASSES
         * All this functionality should probably be parceled out so that the PBackend source file isn't
         * too long and ungainly.
         *
         * We can have something like SceneResourceBaker, with either subclasses or separate smaller classes that are
         * controlled by the SceneResourceBaker; we can then also have ShaderModuleBaker, SceneViewBaker, and
         * SceneRenderGraphBaker classes which the backend would trigger.
         *
         * I really want to keep things as simple and bite-sized as possible
         */

        void bakeSceneResources();

        void bakeSceneShaderConstants();

        void bakeShaderConstant(const std::shared_ptr<ShaderConstant> &sharedPtr);

        static VkShaderStageFlags
        getVkShaderStageFlagsFromShaderConstantShaderStages(
                const std::vector<ShaderConstant::ShaderStage> &shaderConstantShaderStages);

        void bakeSceneImages();

        void bakeImage(const std::shared_ptr<Image> &image);

        void bakeSceneTextures();

        void bakeTexture(const std::shared_ptr<Texture> &sharedPtr);

        void bakeSceneModels();

        void bakeModel(const std::shared_ptr<Model> &model);

        void bakeSceneMaterials();

        void bakeMaterial(const std::shared_ptr<Material> &material);

        void bakeShaderModules();

        void bakeSceneViews();

        void bakeSceneRenderGraph();

        [[maybe_unused]] static VkBufferUsageFlags convertToVkBufferUsageFlags(unsigned int flags);

        static VkFormat convertToVulkanImageFormat(TexelFormat &format);

        static VkExtent3D getVkExtent3DFromImageExtent(Image::ImageExtent2D extent2D);

        static VkSampleCountFlagBits getVkSampleCountFromNumberOfSamples(unsigned int samples);

        static VkImageUsageFlags convertToVkImageUsageFlags(const std::unordered_set<Image::ImageUsage> &imageUsages);

        void bakeSceneBuffers();

        void groupSceneBuffersByBufferUsage(std::vector<std::shared_ptr<Buffer>> &vertexIndexBuffers,
                                            std::vector<std::shared_ptr<Buffer>> &uniformBuffers,
                                            std::vector<std::shared_ptr<Buffer>> &texelBuffers,
                                            std::vector<std::shared_ptr<Buffer>> &storageBuffers) const;

        static VkSamplerCreateInfo bakeVulkanSamplerCreateInfoFromTexture(const std::shared_ptr<Texture> &sharedPtr);

        static VkImageCreateInfo bakeVulkanImageCreateInfoFromTexture(const std::shared_ptr<Texture> &texture);

        static VkFilter getTextureMinificationFilter(const std::shared_ptr<Texture> &texture);

        static VkFilter getTextureMagnificationFilter(const std::shared_ptr<Texture> &texture);

        static VkSamplerMipmapMode getSamplerMipmapMode(const std::shared_ptr<Texture> &texture);

        static VkSamplerAddressMode getSamplerAddressMode(const std::shared_ptr<Texture> &texture);

        static float getSamplerMipLodBias(const std::shared_ptr<Texture> &texture);

        static VkBool32 getSamplerAnisotropyEnableFlag(const std::shared_ptr<Texture> &texture);

        static float getSamplerMaxAnisotropy(const std::shared_ptr<Texture> &texture);

        static VkBool32
        getSamplerCompareEnableFlag(const std::shared_ptr<Texture> &texture);

        static VkCompareOp getSamplerCompareOp(const std::shared_ptr<Texture> &texture);

        static float getSamplerMinLod(const std::shared_ptr<Texture> &texture);

        static float getSamplerMaxLod(const std::shared_ptr<Texture> &texture);

        static VkBorderColor getSamplerBorderColor(const std::shared_ptr<Texture> &texture);

        static VkBool32 getSamplerUnnormalizedCoordinatesFlag(const std::shared_ptr<Texture> &texture);

        void bakeRenderable(const std::shared_ptr<Renderable> &renderable);

        void bakeTriangleMesh(const std::shared_ptr<TriangleMesh> &triangleMeshToBake);

        void
        bakeIndexedTriangleMesh(const std::shared_ptr<PGraphics::IndexedTriangleMesh> &indexedTriangleMeshToBake);

        void bakeListOfBuffers(std::vector<std::shared_ptr<Buffer>> &buffers);

        void bakeDescriptorSetAllocators();
    };

}// namespace PGraphics
