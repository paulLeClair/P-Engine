//
// Created by paull on 2024-03-02.
//

#pragma once

#include <memory>
#include "../VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "../../../GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp"
#include "../../../GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"
#include "../VulkanImage/VulkanImage.hpp"
#include "../Model/VulkanModel.hpp"
#include "../VulkanPushConstant/VulkanPushConstant.hpp"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * This is deprecated
     */
    class VulkanResourceRepository {
    public:
        struct CreationInput {
            VkDevice device = VK_NULL_HANDLE;
            uint32_t deviceGraphicsQueueFamilyIndex = 0;
            VmaAllocator allocator = VK_NULL_HANDLE;

            std::vector<gir::BufferIR> bufferGirs = {};
            std::vector<gir::ImageIR> imageGirs = {}; // not used right now
            std::vector<gir::ShaderConstantIR> shaderConstantGirs = {};
            std::vector<gir::model::ModelIR> modelGirs = {};

            uint32_t minUniformBufferAlignment;
        };


        explicit VulkanResourceRepository(const CreationInput &creationInput)
            : minimumUniformBufferAlignment(creationInput.minUniformBufferAlignment), device(creationInput.device),
              queueFamilyIndices({creationInput.deviceGraphicsQueueFamilyIndex}) {
            allocator = creationInput.allocator;

            // initialize buffer suballocators (just uniforms are used right now)
            uniformBufferSuballocator = VulkanBufferSuballocator(
                VulkanBufferSuballocator::CreationInput{
                    allocator,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    {},
                    queueFamilyIndices,
                    creationInput.minUniformBufferAlignment
                }
            );

            // create the buffers
            suballocateBufferGirs(creationInput.bufferGirs);

            // then we create images
            allocateImageGirs(creationInput.imageGirs); // non-swapchain images are temporarily disabled

            // then we set up the shader constants (push constants in vulkan terminology)
            // TODO!
            // allocateShaderConstantGirs(creationInput.shaderConstantGirs);

            // temporary until render graph refactor (well, so is all of this tbh)
            for (auto &model: creationInput.modelGirs) {
                std::vector<gir::BufferIR> initialUniformBuffers = {};
                for (auto &buffer: model.buffers) {
                    switch (buffer.usage) {
                        case (gir::BufferIR::BufferUsage::UNIFORM_BUFFER): {
                            initialUniformBuffers.push_back(buffer);
                            continue;
                        }
                        // TODO -> more buffer usage types
                        default:
                            continue;
                    }
                }

                std::vector<GeometryGIRAttachment> geometryAttachments = {};
                for (auto &drawAttachment: model.drawAttachments) {
                    for (auto &[vertexBuffers, indexBuffers, animation]: drawAttachment.meshAttachments) {
                        if (vertexBuffers.size() != indexBuffers.size()) {
                            // TODO -> proper logging! no throwing!
                            throw std::runtime_error(
                                "Error in VulkanResourceRepository() -> draw attachment sizes mismatch!");
                        }

                        // TODO -> evaluate this, make sure everything's in place...
                        for (unsigned attachmentIndex = 0;
                             attachmentIndex < vertexBuffers.size();
                             attachmentIndex++) {
                            geometryAttachments.emplace_back(
                                GeometryGIRAttachment{
                                    vertexBuffers[attachmentIndex].attachedBuffer,
                                    indexBuffers[attachmentIndex].attachedBuffer,
                                    vertexBuffers[attachmentIndex].vertexCount,
                                    indexBuffers[attachmentIndex].indexCount
                                }
                            );
                        }
                    }
                }

                std::vector<VulkanBufferSuballocationHandle> suballocatedBuffers = {};
                for (auto &buffer: initialUniformBuffers) {
                    auto suballocationHandleOptional = uniformBufferSuballocator.findSuballocation(buffer.uid);

                    if (!suballocationHandleOptional.has_value()) {
                        // TODO -> proper logging!
                        break; // for now, just abort the loop and end up with malformed data
                    }

                    VulkanBufferSuballocationHandle suballocationHandle = *suballocationHandleOptional;
                    suballocatedBuffers.push_back(
                        suballocationHandle
                    );
                }

                models.insert({
                        model.uid,
                        VulkanModel{
                            VulkanModel::CreationInput{
                                model.name,
                                model.uid,
                                allocator,
                                suballocatedBuffers,
                                minimumUniformBufferAlignment,
                                geometryAttachments,
                                queueFamilyIndices,
                                0.4,
                            }
                        }
                    }
                );
            }
        }

        VulkanResourceRepository() = default;

        VulkanModel *obtainModel(const UniqueIdentifier &resourceId) {
            if (!models.contains(resourceId)) {
                return nullptr;
            }
            return &models.at(resourceId);
        }

        [[nodiscard]] VulkanBufferSuballocationHandle obtainBufferHandle(const UniqueIdentifier &resourceId) {
            auto handle = uniformBufferSuballocator.findSuballocation(resourceId);
            if (!handle.has_value()) {
                return {};
            }
            return handle.get();
        }

        void bakeSwapchainImageGirs(const std::vector<gir::ImageIR> &swapchainImageGirs) {
            std::vector<gir::ImageIR> colorAttachmentGirs = {};
            std::vector<gir::ImageIR> depthAttachmentGirs = {};
            for (const auto &imageGir: swapchainImageGirs) {
                switch (imageGir.imageUsage) {
                    // for single-animated-model demo, only supporting color attachments for now
                    case (gir::ImageIR::ImageUsage::COLOR_ATTACHMENT): {
                        colorAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::DEPTH_ATTACHMENT): {
                        depthAttachmentGirs.push_back(imageGir);
                    }
                    default: {
                        // TODO - log!
                    }
                }
            }

            bakeNewVulkanImagesFromGirList(allocator,
                                           device,
                                           queueFamilyIndices,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           colorAttachmentGirs,
                                           colorAttachments);
        }

        VulkanImage *obtainImage(UniqueIdentifier &resourceUid) {
            // TODO -> search other image sub-repositories (maps) as they get added

            for (auto &colorAttachment: colorAttachments) {
                if (colorAttachment.uid == resourceUid) {
                    return &colorAttachment;
                }
            }

            for (auto &depthAttachment: depthImages) {
                if (depthAttachment.uid == resourceUid) {
                    return &depthAttachment;
                }
            }

            return {};
        }

        // buffer suballocators
        VulkanBufferSuballocator uniformBufferSuballocator = {};
        uint32_t minimumUniformBufferAlignment;

    private:
        VkDevice device = VK_NULL_HANDLE;

        std::vector<uint32_t> queueFamilyIndices = {};

        // allocator for creating resources
        VmaAllocator allocator = {};

        // for the single-animated model demo we're not supporting all the image types; that will change
        // NOTE -> when we fully implement the render graph, the specific usages of the images will be encoded there, not here
        std::vector<VulkanImage> colorAttachments = {};

        // depth attachments and images
        std::vector<VulkanImage> depthImages = {};

        // currently unused
        std::vector<VulkanPushConstant> shaderConstants = {};

        std::unordered_map<UniqueIdentifier, VulkanModel> models = {};

        // TODO -  future expansions as needed (incl the other types of modern pipelines eg raytracing, mesh shading, workgroups, so forth)

        void suballocateBufferGirs(const std::vector<gir::BufferIR> &uniformBufferGirs) {
            uniformBufferSuballocator.suballocateBuffers(uniformBufferGirs);
        }

        static VkExtent3D getVkExtentFromImageGir(const gir::ImageIR &imageIR) {
            return VkExtent3D{
                imageIR.width,
                imageIR.height,
                1 // TODO -> support 3D
            };
        }

        static unsigned getNumberOfMipLevelsFromImageGir(const gir::ImageIR &imageIR) {
            return 1; // TODO - implement mip-mapping support
        }

        static unsigned getNumberOfArrayLevelsFromImageGir(const gir::ImageIR &imageIR) {
            return 1; // TODO - implement image arrays
        }

        static VkSampleCountFlagBits getSampleCountFromImageGir(const gir::ImageIR &imageIR) {
            return VK_SAMPLE_COUNT_1_BIT; // TODO - implement MSAA support (I think that's what this is?)
        }

        static VkImageTiling getImageTilingFromImageGir(
            const gir::ImageIR &imageIR) {
            return VK_IMAGE_TILING_OPTIMAL; // TODO - figure out how we determine when other image tilings are needed
        }

        static VkImageLayout getInitialImageLayoutFromImageGir(const gir::ImageIR &value) {
            // TODO - add some more complex logic (backed by a bit of research) for choosing a good initial layout
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        /**
         * ATOW, this only builds depth attachment images (we'll expand to the other applicable
         * image resource types once the single-animated-model demo is done)
         * @param imageGirs
         */
        void allocateImageGirs(const std::vector<gir::ImageIR> &imageGirs) {
            std::vector<gir::ImageIR> depthAttachmentGirs = {};
            for (const auto &imageGir: imageGirs) {
                switch (imageGir.imageUsage) {
                    case (gir::ImageIR::ImageUsage::DEPTH_STENCIL_ATTACHMENT): {
                        depthAttachmentGirs.push_back(imageGir);
                    }
                    default: {
                        // TODO - log!
                    }
                }
            }
            bakeNewVulkanImagesFromGirList(allocator,
                                           device,
                                           queueFamilyIndices,
                                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                           depthAttachmentGirs,
                                           depthImages);
        }

        static void bakeNewVulkanImagesFromGirList(const VmaAllocator &allocator,
                                                   VkDevice device,
                                                   const std::vector<uint32_t> &deviceFamilyQueueIndices,
                                                   const VkImageUsageFlags initialImageUsageFlags, //NOLINT
                                                   const std::vector<gir::ImageIR> &newImageGirs,
                                                   std::vector<VulkanImage> &imageList) {
            VkImageUsageFlags imageUsageFlags = initialImageUsageFlags;
            for (const auto &newImageGir: newImageGirs) {
                if (!newImageGir.disableTransferDestination) {
                    imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                }
                if (!newImageGir.disableTransferSource) {
                    imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                }

                // since swapchain images are not constructed directly and are used in a unique way (atow), we'll skip em
                if (newImageGir.imageUsage == gir::ImageIR::ImageUsage::SWAPCHAIN_IMAGE) continue;

                imageList.emplace_back(VulkanImage::CreationInput{
                    newImageGir.name,
                    newImageGir.uid,
                    device,
                    VK_IMAGE_TYPE_2D,
                    (VkFormat) newImageGir.imageFormat, // NOTE -> assumes 1-1 correspondence with VkFormat
                    getVkExtentFromImageGir(newImageGir),
                    getNumberOfMipLevelsFromImageGir(newImageGir),
                    getNumberOfArrayLevelsFromImageGir(newImageGir),
                    getSampleCountFromImageGir(newImageGir),
                    getImageTilingFromImageGir(newImageGir),
                    imageUsageFlags,
                    // TODO - change this so it's not hardcoded to use graphics queue
                    deviceFamilyQueueIndices,
                    getInitialImageLayoutFromImageGir(newImageGir),
                    VMA_MEMORY_USAGE_AUTO, // TODO - add configurability for this as needed
                    allocator
                });
            }
        }

        static VkShaderStageFlags getShaderStageFlagsFromShaderConstantGir(
            const std::shared_ptr<gir::ShaderConstantIR> &shaderConstantGir) {
            // TODO - represent shader constant stage usages? pretty simple to throw in a simple enum converter
            return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // just hardcoding this for now
        }

        void allocateShaderConstantGirs(
            const std::vector<gir::ShaderConstantIR> &shaderConstantGirs) {
        }
    };
}
