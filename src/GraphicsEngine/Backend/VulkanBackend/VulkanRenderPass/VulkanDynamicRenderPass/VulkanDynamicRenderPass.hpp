//
// Created by paull on 2022-06-25.
//

#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>

#include "../../VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.hpp"
#include "../VulkanRenderPass.hpp"
#include "../../Model/VulkanModel.hpp"
#include "../../VulkanDescriptorSetAllocator/ResourceDescriptorBindings.hpp"
#include "../../VulkanProgram/VulkanProgram.hpp"
#include "../../VulkanResourceRepository/VulkanResourceRepository.hpp"


namespace pEngine::girEngine::backend::vulkan {
    // for now, we'll just have dynamic resources stuff be directly integrated here since it's the only non-gui render
    // pass that we currently support
    struct DynamicResourceBinding {
        UniqueIdentifier resourceId;

        std::function<std::vector<uint8_t>()> updateCallback = {};

        float priority = 1.0f;
    };

    struct VulkanDynamicRenderPass final : VulkanRenderPass {
        VulkanDynamicRenderPass(
            std::string name,
            const UniqueIdentifier &uniqueIdentifier,
            const VkDevice &device,
            const VkInstance &instance,
            const VmaAllocator &allocator,
            const std::shared_ptr<pipeline::VulkanGraphicsPipeline> &graphicsPipeline,
            VulkanResourceRepository &repositoryHandle,
            const std::vector<DynamicResourceBinding> &dynamicBuffers,
            const std::vector<DynamicResourceBinding> &dynamicImages,
            const VkRenderingInfo &renderingInfo,
            const std::vector<VkRenderingAttachmentInfo> &swapchainImageAttachmentInfos,
            const VkRenderingAttachmentInfo &depthAttachment,
            const VkImage depthImage,
            const std::vector<std::function<void(VkCommandBuffer)> > &renderPassDrawCallbacks,
            const std::vector<VulkanModel *> &models,
            const std::vector<VertexInputBinding> &inputBindings,
            const std::shared_ptr<VulkanProgram> &program,
            const std::vector<VulkanBufferSuballocationHandle> &boundBuffers,
            VmaAllocation &uniformBufferAllocation,
            const uint32_t graphicsQueueFamilyIndex,
            const uint32_t numberOfFramesInFlight,
            const VkQueue queue
        ) : VulkanRenderPass(inputBindings),
            name(std::move(name)),
            uniqueIdentifier(uniqueIdentifier),
            device(device), instance(instance),
            allocator(allocator),
            graphicsAndPresentQueue(queue),
            resourceRepository(repositoryHandle),
            graphicsPipeline(graphicsPipeline),
            program(program),
            renderingInfo(renderingInfo),
            swapchainImageAttachments(swapchainImageAttachmentInfos),
            depthAttachmentRenderingInfo(depthAttachment),
            depthImage(depthImage),
            renderPassDrawCallbacks(renderPassDrawCallbacks),
            models(models),
            inputBindings(inputBindings),
            boundUniformBuffers(boundBuffers),
            dynamicUniformBuffers(dynamicBuffers),
            dynamicImages(dynamicImages),
            uniformBufferAllocation(uniformBufferAllocation),
            numberOfFramesInFlight(numberOfFramesInFlight) {
            std::vector<descriptor::WriteBinding> writeBindings = {};
            obtainDescriptorSetWriteBindings(writeBindings);

            setupDynamicResourceStagingBuffer(allocator, graphicsQueueFamilyIndex);

            resourceDescriptorBindings = descriptor::ResourceDescriptorBindings{
                writeBindings,
                {},
                {}
            };

            constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            perPassCopySemaphores = {};
            for (uint32_t frameIndex = 0; frameIndex < numberOfFramesInFlight; frameIndex++) {
                perPassCopySemaphores.push_back(VK_NULL_HANDLE);

                if (auto result = vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr,
                                                    &perPassCopySemaphores.back()); result != VK_SUCCESS) {
                    throw std::runtime_error("Error in VulkanDynamicRenderPass() -> failed to create semaphore");
                }
            }
        }

        std::vector<VkSemaphore> perPassCopySemaphores;

        VulkanDynamicRenderPass(const VulkanDynamicRenderPass &other) = default;

        ~VulkanDynamicRenderPass() override = default;


        RenderResult recordRenderingCommands(VkCommandBuffer &preAllocatedRenderCommandBuffer,
                                             uint32_t frameInFlightIndex,
                                             std::vector<VkCommandBuffer> &outputCopyCommandBuffers,
                                             VkCommandPool &commandPool
        ) override {
            if (preAllocatedRenderCommandBuffer == VK_NULL_HANDLE) {
                return RenderResult::FAILURE;
            }
            if (frameInFlightIndex >= numberOfFramesInFlight) {
                return RenderResult::FAILURE;
            }

            // record per-pass copy commands
            if (!recordCopyCommandBuffers(outputCopyCommandBuffers, commandPool)) {
                return RenderResult::FAILURE;
            }

            // 1. bind graphics pipeline
            vkCmdBindPipeline(preAllocatedRenderCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              graphicsPipeline->getHandle());

            // 2. begin rendering (TODO - validation of current state before proceeding)
            beginRendering(preAllocatedRenderCommandBuffer, frameInFlightIndex);

            // 3. iterate over bound geometry (which for the animated model demo is just a singular static model list)
            // and submit valid draw calls with the appropriate bindings of descriptor sets etc!

            // 3.b -> I'll try this for the first pass at the descriptor set access interface;
            // in the future there should be multiple descriptor set binding points that are bound at different
            // frequencies, but since we only have need of descriptor sets that are updated each frame, I'll put
            // them here and just iterate over all layouts until we refactor it to support a more robust DS scheme
            for (const auto &model: models) {
                for (auto &layout: program->getDescriptorSetLayouts()) {
                    auto [descriptorSet, descriptorSetIsFresh] = program->requestDescriptorSet(
                        layout, resourceDescriptorBindings);

                    if (descriptorSet == VK_NULL_HANDLE) {
                        return RenderResult::FAILURE;
                    }

                    // if descriptor set is fresh we have to bind resources to it
                    if (descriptorSetIsFresh) {
                        std::vector<VkWriteDescriptorSet> writes = resourceDescriptorBindings.
                                convertToVkWriteDescriptorSets();
                        uint32_t bindingIndex = 0;
                        std::vector<VulkanBufferSuballocationHandle> uniformBufferSuballocationHandles = model->
                                getSuballocatedUniformBuffers();
                        std::vector<VkDescriptorBufferInfo>
                                uniformBufferInfos(uniformBufferSuballocationHandles.size());
                        for (auto &write: writes) {
                            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write.pNext = nullptr;
                            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO -> obtain this from user
                            write.dstSet = descriptorSet;
                            write.descriptorCount = 1; // todo -> support groups of descriptors


                            // NOTE -> this assumes that the uniform buffer suballocations are 1-1 with bindings
                            auto handle = uniformBufferSuballocationHandles.at(bindingIndex);
                            // TODO -> support other descriptor infos when we support textures/etc
                            uniformBufferInfos[bindingIndex].buffer = handle.parentBuffer;
                            uniformBufferInfos[bindingIndex].offset = handle.suballocation.globalBufferOffset;
                            uniformBufferInfos[bindingIndex].range = handle.suballocation.size;
                            write.pBufferInfo = &uniformBufferInfos[bindingIndex];

                            bindingIndex++;
                        }
                        std::vector<VkCopyDescriptorSet> copies = resourceDescriptorBindings.
                                convertToVkCopyDescriptorSets();

                        // update per-frame descriptors
                        vkUpdateDescriptorSets(device, writes.size(), writes.data(), copies.size(), copies.data());
                    }

                    vkCmdBindDescriptorSets(
                        preAllocatedRenderCommandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        graphicsPipeline->getPipelineLayout(),
                        0,
                        1,
                        &descriptorSet,
                        0,
                        nullptr
                    );
                }

                // 3.c -> bind vertex and index buffers, render each mesh in the model
                std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > vertexBufferSuballocationHandles =
                        model->getVertexBufferSuballocations();
                std::vector<std::pair<VulkanBufferSuballocationHandle, uint32_t> > indexBufferSuballocationHandles =
                        model->getIndexBufferSuballocations();
                for (uint32_t drawBindingIndex = 0;
                     drawBindingIndex < vertexBufferSuballocationHandles.size()
                     && drawBindingIndex < indexBufferSuballocationHandles.size(); drawBindingIndex++) {
                    auto [vertexBufferHandle, vertexCount] = vertexBufferSuballocationHandles.at(drawBindingIndex);
                    auto [indexBufferHandle, indexCount] = indexBufferSuballocationHandles.at(drawBindingIndex);
                    vkCmdBindVertexBuffers(
                        preAllocatedRenderCommandBuffer,
                        0,
                        1,
                        &model->getVertexBufferSuballocator().getBufferHandle(),
                        &vertexBufferHandle.suballocation.globalBufferOffset
                    );
                    vkCmdBindIndexBuffer(
                        preAllocatedRenderCommandBuffer,
                        model->getIndexBufferSuballocator().getBufferHandle(),
                        indexBufferHandle.suballocation.globalBufferOffset,
                        VK_INDEX_TYPE_UINT32 // TODO -> acquire the particular index type dynamically
                    );

                    // 3.d -> record the actual draw commands
                    vkCmdDrawIndexed(
                        preAllocatedRenderCommandBuffer,
                        indexCount,
                        1,
                        0,
                        0,
                        0
                    );
                }
            }

            // 4. finish rendering
            vkCmdEndRendering(preAllocatedRenderCommandBuffer);

            return RenderResult::SUCCESS;
        }

        std::string name;
        UniqueIdentifier uniqueIdentifier;

        VkDevice device;
        VkInstance instance;

        VmaAllocator allocator;

        VkQueue graphicsAndPresentQueue = VK_NULL_HANDLE;

        VulkanResourceRepository &resourceRepository;

        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
        std::unordered_map<UniqueIdentifier, uint32_t> stagingBufferOffsets = {};
        uint32_t stagingBufferSize = 0;

        std::shared_ptr<pipeline::VulkanGraphicsPipeline> graphicsPipeline; // TODO -> purge this shared ptr

        std::shared_ptr<VulkanProgram> program;

        VkRenderingInfo renderingInfo = {};
        std::vector<VkRenderingAttachmentInfo> swapchainImageAttachments = {};
        VkRenderingAttachmentInfo depthAttachmentRenderingInfo = {};
        VkImage depthImage = VK_NULL_HANDLE;

        // this may be deprecated but I could see it coming back into usage
        [[maybe_unused]] std::vector<std::function<void(VkCommandBuffer)> > renderPassDrawCallbacks = {};

        // temporary static models list
        std::vector<VulkanModel *> models;
        std::vector<VertexInputBinding> inputBindings;

        // mainly for descriptor sets we need to be able to produce a ResourceBindings() thing
        std::vector<VulkanBufferSuballocationHandle> boundUniformBuffers = {};

        // TEMP: all resource updates are per-pass and only supported by this vulkan dynamic render pass thing
        std::vector<DynamicResourceBinding> dynamicUniformBuffers = {};
        std::vector<DynamicResourceBinding> dynamicImages = {};

        descriptor::ResourceDescriptorBindings resourceDescriptorBindings;
        VmaAllocation &uniformBufferAllocation;

        int frameInFlightIndex = 0;
        uint32_t numberOfFramesInFlight = 0;

    private:
        void setupDynamicResourceStagingBuffer(const VmaAllocator &allocator, const uint32_t graphicsQueueFamilyIndex) {
            for (auto &dynamic_resource_binding: dynamicUniformBuffers) {
                auto bufferItr = std::ranges::find_if(boundUniformBuffers, [&](auto &binding) {
                    return binding.suballocation.resourceId == dynamic_resource_binding.resourceId;
                });
                if (bufferItr == boundUniformBuffers.end()) {
                    // LOG!
                }

                stagingBufferOffsets.insert_or_assign(bufferItr->suballocation.resourceId, stagingBufferSize);
                stagingBufferSize += (*bufferItr).suballocation.size;
            }

            const VkBufferCreateInfo stagingBufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = stagingBufferSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // TODO -> determine whether this is enough; I think it is
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 1,
                .pQueueFamilyIndices = &graphicsQueueFamilyIndex // TODO -> support compute/pure transfer queues
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
        }

        void obtainDescriptorSetWriteBindings(std::vector<descriptor::WriteBinding> &writeBindings) {
            for (auto &buffer: boundUniformBuffers) {
                // TODO -> make sure this works with the new per-mesh pose buffers

                writeBindings.emplace_back(
                    buffer.suballocation.resourceId,
                    buffer.suballocation.bindingIndex,
                    0, // TODO -> support descriptor arrays
                    1,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // TODO -> support other resource types
                    boost::none,
                    boost::optional<VkDescriptorBufferInfo>(
                        VkDescriptorBufferInfo{
                            buffer.parentBuffer,
                            buffer.suballocation.globalBufferOffset,
                            buffer.suballocation.size
                        }
                    ),
                    boost::none
                );
            }
        }

        bool recordCopyCommandBuffers(std::vector<VkCommandBuffer> &outputCopyCommandBuffers,
                                      const VkCommandPool &commandPool) const {
            outputCopyCommandBuffers.clear();
            // first we will obtain the command buffer that will record our copy commands
            VkCommandBufferAllocateInfo copyBufferAllocateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            VkCommandBuffer copyCommandBuffer = VK_NULL_HANDLE;

            auto result = vkAllocateCommandBuffers(device, &copyBufferAllocateInfo, &copyCommandBuffer);
            if (result != VK_SUCCESS) {
                // TODO -> proper logging!
                return false;
            }

            // begin the copy command buffer
            VkCommandBufferBeginInfo beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            result = vkBeginCommandBuffer(copyCommandBuffer, &beginInfo);
            if (result != VK_SUCCESS) {
                return true;
            }

            // NEW STUFF:
            // all per-pass buffer copies will be put into one array I guess
            std::vector<VkBufferCopy2> uniformBufferCopies;
            for (auto &binding: dynamicUniformBuffers) {
                // 1. find resource using the resourceId in the binding
                VulkanBufferSuballocationHandle bufferHandle = {};
                for (auto &bufferSuballocation: boundUniformBuffers) {
                    if (bufferSuballocation.suballocation.resourceId == binding.resourceId) {
                        bufferHandle = bufferSuballocation;
                        break;
                    }
                }
                if (bufferHandle.parentBuffer == VK_NULL_HANDLE) {
                    // TODO -> LOG! for now, if we don't find it just continue
                    continue;
                }

                // 2. call that resource's update callback and store its result
                std::vector<uint8_t> newBufferData = {};
                newBufferData = binding.updateCallback();

                // TODO -> hash the result, compare hashes to determine whether change occurred
                if (newBufferData.empty()) {
                    // TODO -> LOG! for now, just continue if our update function returns an empty array
                    continue;
                }

                if (newBufferData.size() > bufferHandle.suballocation.size) {
                    // TODO -> log! for now if we return too much data we'll just continue
                    continue;
                }

                // 3. copy the result of the update (CPU->CPU) into the host-side staging buffer, and
                // then enqueue a copy command to copy from that region of the staging buffer to the GPU
                const uint32_t stagingBufferOffset = stagingBufferOffsets.at(bufferHandle.suballocation.resourceId);
                const uint32_t newSize = static_cast<uint32_t>(newBufferData.size());
                result = vmaCopyMemoryToAllocation(
                    allocator,
                    newBufferData.data(),
                    stagingBufferAllocation,
                    stagingBufferOffset,
                    newSize
                );
                if (result != VK_SUCCESS) {
                    // TODO -> log!
                    return false;
                }

                uniformBufferCopies.push_back({
                    .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
                    .pNext = nullptr,
                    .srcOffset = stagingBufferOffset,
                    .dstOffset = bufferHandle.suballocation.globalBufferOffset,
                    .size = newSize
                });
            }

            VkCopyBufferInfo2 uniformCopyBufferInfo = {
                .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
                .srcBuffer = stagingBuffer,
                .dstBuffer = resourceRepository.uniformBufferSuballocator.getBufferHandle(),
                .regionCount = static_cast<uint32_t>(uniformBufferCopies.size()),
                .pRegions = uniformBufferCopies.data(),
            };
            vkCmdCopyBuffer2(copyCommandBuffer, &uniformCopyBufferInfo);

            for ([[maybe_unused]] auto &binding: dynamicImages) {
                // TODO -> dynamic images (and also textures in general)
            }

            result = vkEndCommandBuffer(copyCommandBuffer);
            if (result != VK_SUCCESS) {
                // TODO -> proper logging!
                return false;
            }

            // pass copy command buffer into output array so it can be submitted externally to this pass
            outputCopyCommandBuffers.push_back(copyCommandBuffer);
            return true;
        }

        void beginRendering(VkCommandBuffer &preAllocatedRenderCommandBuffer, uint32_t frameInFlightIndex) {
            // 2.a -> set color attachment info
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &swapchainImageAttachments[frameInFlightIndex];

            // 2.b -> set depth attachment info
            depthAttachmentRenderingInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentRenderingInfo.clearValue.depthStencil.depth = 1.0f;
            renderingInfo.pDepthAttachment = &depthAttachmentRenderingInfo;

            vkCmdBeginRendering(preAllocatedRenderCommandBuffer, &renderingInfo);
        }
    };
} // namespace PGraphics
