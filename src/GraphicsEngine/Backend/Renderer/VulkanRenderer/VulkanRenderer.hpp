//
// Created by paull on 2022-06-19.
//

#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <memory>
#include <vector>

#include "../../ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp"
#include "../../VulkanBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp"
#include "../../VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp"
#include "../Renderer.hpp"
#include "../VulkanGuiThread/VulkanGuiThread.hpp"

// ugly macro for now
#define DEFAULT_SWAPCHAIN_TIMEOUT_IN_MS 5

namespace pEngine::girEngine::backend::vulkan {
    struct Frame;

    class VulkanRenderer final : public Renderer {
    public:
        struct CreationInput {
            // TODO -> factor out the individual items that are used so we don't pass a shared pointer in for no reason
            std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext;

            VmaAllocator allocator;
            VulkanPresentationEngine &presentationEngine;

            // TODO - consider relocating the presentation engine so we dont have to configure the swapchain here
            VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            VkCompositeAlphaFlagBitsKHR swapchainCompositeAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            std::vector<float> clearColor = {0.0, 0.0, 0.0, 0.0};

            boolean guiThreadEnabled = true;
            std::vector<std::function<void()> > guiThreadCallbacks = {};

            uint32_t numberOfFramesInFlight = 2;
        };

        explicit VulkanRenderer(const CreationInput &createInfo);


        ~VulkanRenderer() override;

        void newImguiFrame() const {
            if (guiThreadIsEnabled && guiThread) {
                guiThread->beginNewImguiFrame();
            }
        }

        enum class RenderFrameResult {
            SUCCESS,
            FAILURE
        };

        /**
         * Hardcoding this to true for now
         */
        const bool guiThreadIsEnabled = true;

        /**
         * Current frame count
         */
        long frameCount = 0;

        void setGuiCallbacks(std::vector<std::function<void()> > &&callbacks) const {
            if (!guiThreadIsEnabled) {
                // TODO -> log!
                return;
            }
            guiThread->setCallbacks(std::move(callbacks));
        }


        RenderFrameResult renderFrame();

        std::vector<float> &acquireClearColorHandle() {
            return clearColor;
        }

        uint32_t getCurrentFrameIndex() const {
            return currentFrameIndex;
        }

        void initializeRenderData(const std::vector<std::shared_ptr<VulkanRenderPass> > &passes,
                                  const VulkanResourceRepository *repository);

        [[nodiscard]] VulkanPresentationEngine &getPresentationEngine() const {
            if (!presentationEngine) {
                // TODO -> log!
                throw std::runtime_error(
                    "Error in VulkanRenderer::getPresentationEngine() -> VulkanPresentationEngine handle is null");
            }
            return *presentationEngine;
        }

        [[nodiscard]] std::shared_ptr<appContext::vulkan::VulkanApplicationContext> getApplicationContext() const {
            return applicationContext;
        }

        [[nodiscard]] VkFence getPreviousFrameFence() const;

        [[nodiscard]] VkFence getCurrentFrameFence() const;

        void recordRenderGraphGuiPipelineBarrierCommandBuffer(VkCommandBuffer &buffer);

        /**
         * The set of wait semaphores that signify *all* previous rendering has completed for a given submission.
         * @return All semaphores that must be waited on in order to proceed after all submissions have completed in the correct order
         */
        std::vector<VkSemaphore> constructAndSubmitRenderCommands();

        void allocateAndRecordFrameInitCommandBuffer(VkCommandBuffer &frameInitCommandBuffer) const;

        std::vector<std::unique_ptr<Frame> > frames;

    private:
        std::shared_ptr<appContext::vulkan::VulkanApplicationContext> applicationContext;

        // for the animated model demo I'll try to just have the renderer create the presentation engine itself
        VulkanPresentationEngine *const presentationEngine;

        std::vector<std::shared_ptr<VulkanRenderPass> > renderPasses = {};
        std::vector<VkSemaphore> perPassCopySemaphores = {};

        VmaAllocator allocator;

        // these semaphores should be used to signal when the renderer has acquired an image from the swapchain;
        // they are separated out to be 1-1 with the number of frames so that we can share them without knowing
        // a priori which frame index is next
        std::vector<VkSemaphore> imageAcquiredSemaphores;
        uint32_t currentImageAcquiredSemaphore = 0;

        // for simplicity, I'll have the renderer maintain its own pool for submitting certain "overhead" commands
        // that aren't part of the user's render passes
        VkCommandPool commandPool;

        // this just controls what the clear color will be before a frame is drawn
        std::vector<float> clearColor;

        std::unique_ptr<gui::VulkanGuiThread> guiThread = nullptr;

        uint32_t previousFrameIndex = 0;
        uint32_t currentFrameIndex = 0;

        void recordInitialImageTransitionCommands(VkCommandBuffer initialImageTransitionCommandBuffer) const;

        void recordEndOfFrameSwapchainImageTransition(VkCommandBuffer &endOfFrameCommandBuffer,
                                                      const VkImageMemoryBarrier2 &finalSwapchainImageTransitionBarrier)
        const;

        void freePreviouslyUsedCommandBuffers(VkCommandPool commandPool) const;

        static
        void prepareAcquiredSwapchainImageInitialTransitionBarrier(const VkImage &acquiredSwapchainImage,
                                                                   VkImageMemoryBarrier2 &
                                                                   acquiredSwapchainImageBarrier) {
            acquiredSwapchainImageBarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                nullptr,
                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, // srcStageMask
                0, // srcAccessMask
                VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT, // dstStageMask -> don't clear until we acquire the image
                VK_ACCESS_2_TRANSFER_WRITE_BIT, // dstAccessMask -> after we acquire it, allow clear access
                VK_IMAGE_LAYOUT_UNDEFINED, // we don't care about whatever was in here before
                VK_IMAGE_LAYOUT_GENERAL, // since we're clearing before any drawing happens, we want general layout
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                acquiredSwapchainImage,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    VK_REMAINING_MIP_LEVELS,
                    0,
                    VK_REMAINING_ARRAY_LAYERS
                }
            };
        }

        void
        prepareAcquiredSwapchainImageFinalTransitionBarrier(const VkImage &swapchainImage,
                                                            VkImageMemoryBarrier2 &barrier) const {
            barrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                nullptr,
                // we want all color attachment output to finish
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                // we want to make all color attachment writes available
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                applicationContext->getGraphicsQueueFamilyIndex(),
                applicationContext->getGraphicsQueueFamilyIndex(),
                swapchainImage,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    VK_REMAINING_MIP_LEVELS,
                    0,
                    VK_REMAINING_ARRAY_LAYERS
                }
            };
        }

        static void
        recordCommandBufferWithImageMemoryBarrier(const VkCommandBuffer &transitionBuffer,
                                                  VkImageMemoryBarrier2 imageTransitionBarrier) {
            const VkDependencyInfo dependencyInfo{
                VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                nullptr,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &imageTransitionBarrier
            };

            vkCmdPipelineBarrier2(transitionBuffer, &dependencyInfo);
        }


        void recordCommandBufferWithClearCommand(const VkCommandBuffer &buffer, VkImage swapchainImage) const;

        void
        recordFrameInitBarriers(const VkCommandBuffer &buffer,
                                const VkImage &swapchainImage) const;


        void copyRenderPassStaticResourcesToGPU(const VulkanResourceRepository *resourceRepository) const {
            if (renderPasses.empty()) {
                //TODO -> log!
                return;
            }

            std::unordered_map<UniqueIdentifier, uint32_t> stagingBufferOffsets = {};
            uint32_t stagingBufferSize = 0;
            VkBuffer stagingBuffer = VK_NULL_HANDLE;
            VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

            // NEW: we'll just have the renderer make its own temporary staging buffer here since we're waiting on a fence anyway
            for (const auto &renderPass: renderPasses) {
                for (auto vulkan_dynamic_render_pass = std::dynamic_pointer_cast<VulkanDynamicRenderPass>(renderPass);
                     auto &model: vulkan_dynamic_render_pass->models) {
                    for (auto &uniformBufferBinding: model->getSuballocatedUniformBuffers()) {
                        if (std::ranges::any_of(vulkan_dynamic_render_pass->dynamicUniformBuffers,
                                                [&](const auto &dynamicUniformBufferBinding) {
                                                    return uniformBufferBinding.suballocation.resourceId ==
                                                           dynamicUniformBufferBinding.resourceId;
                                                })) {
                            continue;
                        }

                        stagingBufferOffsets.insert_or_assign(uniformBufferBinding.suballocation.resourceId,
                                                              stagingBufferSize);
                        stagingBufferSize += uniformBufferBinding.suballocation.size;
                    }

                    for (const auto &vertexBufferSuballocation:
                         model->getVertexBufferSuballocations() | std::views::keys) {
                        stagingBufferOffsets.insert_or_assign(vertexBufferSuballocation.suballocation.resourceId,
                                                              stagingBufferSize);
                        stagingBufferSize += vertexBufferSuballocation.suballocation.size;
                    }

                    for (const auto &indexBufferSuballocation:
                         model->getIndexBufferSuballocations() | std::views::keys) {
                        stagingBufferOffsets.insert_or_assign(indexBufferSuballocation.suballocation.resourceId,
                                                              stagingBufferSize);
                        stagingBufferSize += indexBufferSuballocation.suballocation.size;
                    }
                }
            }

            uint32_t graphics_queue_family_index = applicationContext->getGraphicsQueueFamilyIndex();
            const VkBufferCreateInfo stagingBufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = stagingBufferSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // TODO -> determine whether this is enough; I think it is
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 1,
                .pQueueFamilyIndices = &graphics_queue_family_index
                // TODO -> support compute/pure transfer queues
            };

            // these should hopefully work for a staging buffer (comes directly from VMA docs)
            VmaAllocationCreateInfo stagingBufferAllocationCreateInfo = {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO,
            };

            VmaAllocationInfo stagingBufferAllocationInfo = {
                // this can be left blank
            };

            const auto stagingBufferCreationResult
                    = vmaCreateBuffer(allocator,
                                      &stagingBufferCreateInfo,
                                      &stagingBufferAllocationCreateInfo,
                                      &stagingBuffer,
                                      &stagingBufferAllocation,
                                      &stagingBufferAllocationInfo);
            if (stagingBufferCreationResult != VK_SUCCESS) {
                // TODO -> proper logging tools!
                throw std::runtime_error(
                    "Error in VulkanDynamicRenderPass() -> failed to create staging buffer");
            }

            VkCommandBuffer staticGeometryCommandBuffer = VK_NULL_HANDLE;
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
            auto result = vkAllocateCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(),
                                                   &commandBufferAllocateInfo,
                                                   &staticGeometryCommandBuffer);
            if (result != VK_SUCCESS) {
                // TODO -> log!
                return;
            }

            auto beginInfo = VkCommandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                .pInheritanceInfo = nullptr
            };
            result = vkBeginCommandBuffer(staticGeometryCommandBuffer, &beginInfo);
            if (result != VK_SUCCESS) {
                // TODO -> log!
                return;
            }

            for (const auto &pass: renderPasses) {
                // TODO -> filter based on render pass type
                std::shared_ptr<VulkanDynamicRenderPass> dynamicPass = std::dynamic_pointer_cast<
                    VulkanDynamicRenderPass>(pass);

                for (const auto &model: dynamicPass->models) {
                    std::vector<VkBufferCopy2> uniformBufferCopies = {};
                    for (const auto &buffer: model->getSuballocatedUniformBuffers()) {
                        // we want to only copy this over if it's not a dynamic resource
                        // TODO -> consider tracking dynamic resources in a set as opposed to a vector
                        if (std::ranges::any_of(dynamicPass->dynamicUniformBuffers, [&](const auto &dynamicBuffer) {
                            return dynamicBuffer.resourceId == buffer.suballocation.resourceId;
                        })) {
                            continue;
                        }

                        VkDeviceSize uniformBufferStagingBufferOffset = stagingBufferOffsets[buffer.suballocation.
                            resourceId];
                        result = vmaCopyMemoryToAllocation(allocator, buffer.suballocation.currentData->data(),
                                                           stagingBufferAllocation,
                                                           uniformBufferStagingBufferOffset, buffer.suballocation.size);
                        if (result != VK_SUCCESS) {
                            // TODO -> log!
                            return;
                        }

                        uniformBufferCopies.emplace_back(
                            VK_STRUCTURE_TYPE_BUFFER_COPY_2,
                            nullptr,
                            stagingBufferOffsets.at(buffer.suballocation.resourceId),
                            buffer.suballocation.globalBufferOffset,
                            buffer.suballocation.size
                        );
                    }

                    // here we'll copy the vertex and index buffer data into our staging buffer and then GPU
                    std::vector<VkBufferCopy2> vertexBufferCopies = {};
                    for (const auto &buffer: model->getVertexBufferSuballocations() | std::views::keys) {
                        VkDeviceSize vertexBufferStagingBufferOffset = stagingBufferOffsets.at(
                            buffer.suballocation.resourceId);

                        result = vmaCopyMemoryToAllocation(allocator,
                                                           buffer.suballocation.currentData->data(),
                                                           stagingBufferAllocation,
                                                           vertexBufferStagingBufferOffset,
                                                           buffer.suballocation.size);
                        if (result != VK_SUCCESS) {
                            // TODO -> log!
                            return;
                        }

                        vertexBufferCopies.push_back(VkBufferCopy2{
                            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
                            .pNext = nullptr,

                            .srcOffset = vertexBufferStagingBufferOffset,
                            .dstOffset = buffer.suballocation.globalBufferOffset,
                            .size = buffer.suballocation.size,
                        });
                    }

                    std::vector<VkBufferCopy2> indexBufferCopies = {};
                    for (const auto &buffer: model->getIndexBufferSuballocations() | std::views::keys) {
                        VkDeviceSize indexBufferStagingBufferOffset = stagingBufferOffsets.at(
                            buffer.suballocation.resourceId);
                        result = vmaCopyMemoryToAllocation(allocator,
                                                           buffer.suballocation.currentData->data(),
                                                           stagingBufferAllocation,
                                                           indexBufferStagingBufferOffset,
                                                           buffer.suballocation.size);

                        if (result != VK_SUCCESS) {
                            // TODO -> log!
                            return;
                        }

                        indexBufferCopies.push_back(VkBufferCopy2{
                            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
                            .pNext = nullptr,
                            .srcOffset = indexBufferStagingBufferOffset,
                            .dstOffset = buffer.suballocation.globalBufferOffset,
                            .size = buffer.suballocation.size,
                        });
                    }

                    if (uniformBufferCopies.size() > 0) {
                        VkCopyBufferInfo2 uniformBufferCopyInfo = {
                            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
                            .pNext = nullptr,
                            .srcBuffer = stagingBuffer,
                            .dstBuffer = resourceRepository
                                             ? resourceRepository->uniformBufferSuballocator.getBufferHandle()
                                             : VK_NULL_HANDLE,
                            .regionCount = static_cast<uint32_t>(uniformBufferCopies.size()),
                            .pRegions = uniformBufferCopies.data(),

                        };
                        vkCmdCopyBuffer2(staticGeometryCommandBuffer, &uniformBufferCopyInfo);
                    }

                    VkCopyBufferInfo2 vertexBufferCopyInfo{
                        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
                        .srcBuffer = stagingBuffer,
                        .dstBuffer = model->getVertexBufferSuballocator().getBufferHandle(),
                        .regionCount = static_cast<uint32_t>(vertexBufferCopies.size()),
                        .pRegions = vertexBufferCopies.data(),
                    };
                    vkCmdCopyBuffer2(staticGeometryCommandBuffer, &vertexBufferCopyInfo);

                    VkCopyBufferInfo2 indexBufferCopyInfo{
                        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
                        .srcBuffer = stagingBuffer,
                        .dstBuffer = model->getIndexBufferSuballocator().getBufferHandle(),
                        .regionCount = static_cast<uint32_t>(indexBufferCopies.size()),
                        .pRegions = indexBufferCopies.data(),
                    };
                    vkCmdCopyBuffer2(staticGeometryCommandBuffer, &indexBufferCopyInfo);
                }
            }

            // after the commands are enqueued in our buffer we submit that bad boy
            result = vkEndCommandBuffer(staticGeometryCommandBuffer);
            if (result != VK_SUCCESS) {
                // TODO -> log!
                return;
            }

            VkCommandBufferSubmitInfo commandBufferSubmitInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .pNext = nullptr,
                .commandBuffer = staticGeometryCommandBuffer,
                .deviceMask = 0,
            };

            VkSubmitInfo2 staticGeometrySubmitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .waitSemaphoreInfoCount = 0,
                .pWaitSemaphoreInfos = nullptr,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &commandBufferSubmitInfo,
                .signalSemaphoreInfoCount = 0,
                .pSignalSemaphoreInfos = nullptr,
            };

            VkFence staticGeometrySubmitFence = VK_NULL_HANDLE;
            VkFenceCreateInfo staticGeometrySubmitFenceInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            result = vkCreateFence(applicationContext->getLogicalDevice()->getVkDevice(),
                                   &staticGeometrySubmitFenceInfo, nullptr, &staticGeometrySubmitFence);
            if (result != VK_SUCCESS) {
                // TODO -> log!
                return;
            }

            result = vkQueueSubmit2(applicationContext->getGraphicsQueue(), 1, &staticGeometrySubmitInfo,
                                    staticGeometrySubmitFence);
            if (result != VK_SUCCESS) {
                // TODO-> log!
                return;
            }

            result = vkWaitForFences(applicationContext->getLogicalDevice()->getVkDevice(), 1,
                                     &staticGeometrySubmitFence, VK_TRUE, UINT64_MAX);
            if (result != VK_SUCCESS) {
                //TODO -> log!
            }

            // once submission has completed we can free the command buffer here
            vkFreeCommandBuffers(applicationContext->getLogicalDevice()->getVkDevice(), commandPool, 1,
                                 &staticGeometryCommandBuffer);
        }
    };
}
