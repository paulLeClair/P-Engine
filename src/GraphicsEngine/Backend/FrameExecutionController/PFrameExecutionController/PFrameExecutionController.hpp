//
// Created by paull on 2022-06-19.
//

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <memory>
#include <utility>
#include <vector>

#include "../../../../EngineCore/ThreadPool/PThreadPool/PThreadPool.hpp"
#include "../../../../lib/vk_mem_alloc/vk_mem_alloc.h"
#include "../../../Scene/SceneResources/SceneResource.hpp"
#include "../../../Scene/ShaderModule/PShaderModule/PShaderModule.hpp"
#include "../../../Scene/ShaderModule/ShaderModule.hpp"
#include "../../ApplicationContext/PApplicationContext/PApplicationContext.hpp"
#include "../../PBackend/VulkanBuffer/VulkanBuffer.hpp"
#include "../../PBackend/VulkanImage/VulkanImage.hpp"
#include "../../PBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp"
#include "../../PBackend/VulkanShaderModule/VulkanShaderModule.hpp"
#include "../../ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.hpp"
#include "../FrameExecutionController.hpp"
#include "../Frame/PFrame/PFrame.hpp"
#include "../../../Scene/PScene/PScene.hpp"


namespace PGraphics {

    // TODO - break up the FrameExecutionController class (or keep it and subdivide functionality) into separate, smaller, more focused classes

    class PFrameExecutionController : public FrameExecutionController {
    public:
        struct CreationInput {
            std::shared_ptr<PApplicationContext> applicationContext;
            std::shared_ptr<PScene> scene;
            std::shared_ptr<PEngine::PThreadPool> threadPool;
            unsigned int numBufferedFrames = 3;
        };

        explicit PFrameExecutionController(const CreationInput &createInfo);

        PFrameExecutionController(const PFrameExecutionController &other)
                : numBufferedFrames(other.numBufferedFrames),
                  currentFrameIndex(other.currentFrameIndex),
                  scene(other.scene),
                  threadPool(other.threadPool),
                  applicationContext(other.applicationContext),
                  allocator(VK_NULL_HANDLE),
                  acquireSwapchainImageFence(VK_NULL_HANDLE) {
            initializeSwapchainImageAcquisitionFence();
            initializeVmaAllocator();
        }

        ~PFrameExecutionController() = default;

        void executeNextFrame() override;

        [[nodiscard]] unsigned int getCurrentFrameIndex() const override {
            return currentFrameIndex;
        }

        void initializeFrames();

    private:
        const unsigned int numBufferedFrames;
        unsigned int currentFrameIndex;

        // TODO - update the ThreadPool interface so we aren't using PThreadPool directly here
        std::shared_ptr<PEngine::PThreadPool> threadPool;

        std::shared_ptr<Scene> scene;

        std::shared_ptr<PApplicationContext> applicationContext;
        std::shared_ptr<PWindowSystem> windowSystem;

        VmaAllocator allocator;

        std::vector<PFrame> frames;

        VkFence acquireSwapchainImageFence;

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

        void updateActiveBufferedFrameIndex();

        void initializeSwapchainImageAcquisitionFence();

    };

}// namespace PGraphics