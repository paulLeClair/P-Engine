//
// Created by paull on 2024-03-02.
//

#pragma once

#include <memory>
#include "../VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"
#include "../../../GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp"
#include "../../../GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"
#include "../VulkanImage/VulkanImage.hpp"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * Idea: to prevent having to pass in a handle to the backend for this functionality,
     * I'll make this class be responsible for hanging on to the resources in an active
     * Vulkan rendering context (which I guess is just the entire state of girEngine
     * running a vulkan backend).
     *
     * The idea should be that the "scene" (n2b confused with frontend scene) will have a
     * a bunch of resources that may be created/destroyed etc as execution happens;
     *
     * One thing: should I be having this object just receiving already-created resources?
     * Or should I have this thing act as an allocator or something?
     *
     * Hmm it might make sense to have it be more of an allocator because it's kind of handling
     * that one aspect for the backend itself;
     *
     * That's definitely a solid idea I think and it would make sense to break off the allocation stuff
     * into a class where I can more easily fuck with the allocation process itself; for this current bout of work
     * it may make sense to just throw in some relatively-empty interface methods that allow you to acquire
     * a resource by its UID or w/e
     *
     */
    class VulkanResourceRepository {
    public:
        struct CreationInput {
            appContext::vulkan::VulkanLogicalDevice &device;
            VmaAllocator allocator;

            std::vector<std::shared_ptr<gir::BufferIR> > bufferGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > imageGirs = {};
            std::vector<std::shared_ptr<gir::ShaderConstantIR> > shaderConstantGirs = {};
        };


        explicit VulkanResourceRepository(const CreationInput &creationInput)
                : device(creationInput.device), allocator(creationInput.allocator) {
            // first we create the buffers
            suballocateBufferGirs(creationInput.bufferGirs);

            // then we create images
            allocateImageGirs(creationInput.imageGirs);

            // then we set up the shader constants
            allocateShaderConstantGirs(creationInput.shaderConstantGirs);

            // TODO - any other resources we want to stick in this classs
        }

        [[nodiscard]] const VulkanBufferSuballocator &getUniformBufferSuballocator() const {
            return *uniformBufferSuballocator;
        }


        [[nodiscard]] const VulkanBufferSuballocator &getIndexBufferSuballocation() const {
            return *indexBufferSuballocator;
        }

        [[nodiscard]] const VulkanBufferSuballocator &getVertexBufferSuballocation() const {
            return *vertexBufferSuballocator;
        }

        [[nodiscard]] const VulkanBufferSuballocator &getTexelBufferSuballocation() const {
            return *texelBufferSuballocator;
        }

        [[nodiscard]] const VulkanBufferSuballocator &getStorageBufferSuballocation() const {
            return *storageBufferSuballocator;
        }

    private:
        appContext::vulkan::VulkanLogicalDevice &device;

        // allocator for creating resources
        VmaAllocator allocator;

        // buffer suballocators
        std::shared_ptr<VulkanBufferSuballocator> uniformBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> indexBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> vertexBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> texelBufferSuballocator;
        std::shared_ptr<VulkanBufferSuballocator> storageBufferSuballocator;

        // not entirely sure how we want to store these images but simple vectors will do for now
        std::vector<std::shared_ptr<VulkanImage> > storageImages;
        std::vector<std::shared_ptr<VulkanImage> > sampledImages;
        std::vector<std::shared_ptr<VulkanImage> > depthAttachments;
        std::vector<std::shared_ptr<VulkanImage> > depthStencilAttachments;
        std::vector<std::shared_ptr<VulkanImage> > stencilAttachments;
        std::vector<std::shared_ptr<VulkanImage> > colorAttachments;
        std::vector<std::shared_ptr<VulkanImage> > inputAttachments;

        // might change this too of course
        std::vector<std::shared_ptr<VulkanPushConstant> > shaderConstants;

        // TODO - we forgot about combined image-samplers! add that in

        // TODO -  future expansions as needed (raytracing might need some new types)

        void suballocateBufferGirs(const std::vector<std::shared_ptr<gir::BufferIR> > &bufferGirs) const {
            std::vector<std::shared_ptr<gir::BufferIR> > uniformBufferGirs = {};
            std::vector<std::shared_ptr<gir::BufferIR> > dynamicUniformBufferGirs = {}; // TODO - add support for this
            std::vector<std::shared_ptr<gir::BufferIR> > storageBufferGirs = {};
            std::vector<std::shared_ptr<gir::BufferIR> > dynamicStorageBufferGirs = {}; // TODO - add support for this
            std::vector<std::shared_ptr<gir::BufferIR> > texelBufferGirs = {};
            std::vector<std::shared_ptr<gir::BufferIR> > storageTexelBufferGirs = {};
            std::vector<std::shared_ptr<gir::BufferIR> > vertexBufferGirs = {};
            std::vector<std::shared_ptr<gir::BufferIR> > indexBufferGirs = {};
            for (auto &bufferGir: bufferGirs) {
                // this has to be refactored a bit I think... or at least I need to support transfer stuff easily...
                switch (bufferGir->getUsage()) {
                    case (gir::BufferIR::BufferUsage::UNIFORM_BUFFER): {
                        uniformBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::DYNAMIC_UNIFORM_BUFFER): {
                        dynamicUniformBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::STORAGE_BUFFER): {
                        storageBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::DYNAMIC_STORAGE_BUFFER): {
                        dynamicStorageBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::TEXEL_BUFFER): {
                        texelBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::STORAGE_TEXEL_BUFFER): {
                        storageTexelBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::VERTEX_BUFFER): {
                        vertexBufferGirs.push_back(bufferGir);
                        break;
                    }
                    case (gir::BufferIR::BufferUsage::INDEX_BUFFER): {
                        indexBufferGirs.push_back(bufferGir);
                        break;
                    }
                    default: {
                        // TODO - log
                        break;
                    }
                }
            }

            // then we have to actually suballocate all the buffers I guess?
            uniformBufferSuballocator->suballocateBuffers(uniformBufferGirs);
            vertexBufferSuballocator->suballocateBuffers(vertexBufferGirs);
            indexBufferSuballocator->suballocateBuffers(indexBufferGirs);
            storageBufferSuballocator->suballocateBuffers(storageBufferGirs);
            texelBufferSuballocator->suballocateBuffers(texelBufferGirs);
        }

        static VkFormat getVkFormatFromImageGir(const std::shared_ptr<gir::ImageIR> &imageGir) {
            // TODO - add a bunch of IR and Scene enums for different common image formats
            switch (imageGir->getFormat()) {
                // TODO - add in formats when we start actually trying to use image resources lol
                // (not gonna spend a ton of time implementing the formats from VkSpec  all at once)
                default: {
                    // TODO - log
                    return VK_FORMAT_UNDEFINED;
                }
            }
        }

        static VkExtent3D getVkExtentFromImageGir(const std::shared_ptr<gir::ImageIR> &imageIR) {
            return VkExtent3D{
                    imageIR->getWidth(),
                    imageIR->getHeight(),
                    0 // TODO -> support 3D
            };
        }

        static unsigned getNumberOfMipLevelsFromImageGir(const std::shared_ptr<gir::ImageIR> &imageIR) {
            return 1; // TODO - implement mip-mapping support
        }

        static unsigned getNumberOfArrayLevelsFromImageGir(const std::shared_ptr<gir::ImageIR> &imageIR) {
            return 1; // TODO - implement image arrays
        }

        static VkSampleCountFlagBits getSampleCountFromImageGir(const std::shared_ptr<gir::ImageIR> &imageIR) {
            return VK_SAMPLE_COUNT_1_BIT; // TODO - implement MSAA support (I think that's what this is?)
        }

        static VkImageTiling getImageTilingFromImageGir(
                const std::shared_ptr<gir::ImageIR> &imageIR) {
            return VK_IMAGE_TILING_OPTIMAL; // TODO - figure out how we determine when other image tilings are needed
        }

        static VkImageLayout getInitialImageLayoutFromImageGir(const std::shared_ptr<gir::ImageIR> &value) {
            // TODO - add some more complex logic (backed by a bit of research) for choosing a good
            // initial layout, or potentially adding whatever relevant contextual information
            // that we'd need to
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        void allocateImageGirs(const std::vector<std::shared_ptr<gir::ImageIR> > &imageGirs) {
            std::vector<std::shared_ptr<gir::ImageIR> > storageImageGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > sampledImageGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > colorAttachmentGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > inputAttachmentGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > depthAttachmentGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > stencilAttachmentGirs = {};
            std::vector<std::shared_ptr<gir::ImageIR> > depthStencilAttachmentGirs = {};
            for (const auto &imageGir: imageGirs) {
                switch (imageGir->getImageUsage()) {
                    case (gir::ImageIR::ImageUsage::STORAGE_IMAGE): {
                        storageImageGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::SAMPLED_TEXTURE): {
                        sampledImageGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::COLOR_ATTACHMENT): {
                        colorAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::INPUT_ATTACHMENT): {
                        inputAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::DEPTH_STENCIL_ATTACHMENT): {
                        depthStencilAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::STENCIL_ATTACHMENT): {
                        stencilAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    case (gir::ImageIR::ImageUsage::DEPTH_ATTACHMENT): {
                        depthAttachmentGirs.push_back(imageGir);
                        break;
                    }
                    default: {
                        // TODO - log!
                    }
                }
            }

            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_STORAGE_BIT, storageImageGirs,
                                           storageImages);

            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_SAMPLED_BIT, sampledImageGirs,
                                           sampledImages);

            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, colorAttachmentGirs,
                                           colorAttachments);

            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, inputAttachmentGirs,
                                           inputAttachments);

            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                           depthStencilAttachmentGirs, depthStencilAttachments);

            // TODO - consider whether to separate depth and stencil out ? for now maybe combined depth-stencil is okay lol...
            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                           depthAttachmentGirs, depthAttachments);
            bakeNewVulkanImagesFromGirList(allocator, device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                           stencilAttachmentGirs, stencilAttachments);
        }

        static void bakeNewVulkanImagesFromGirList(const VmaAllocator &allocator,
                                                   const appContext::vulkan::VulkanLogicalDevice &device,
                                                   const VkImageUsageFlags initialImageUsageFlags,
                                                   const std::vector<std::shared_ptr<gir::ImageIR> > &newImageGirs,
                                                   std::vector<std::shared_ptr<VulkanImage> > &imageList) {
            VkImageUsageFlags imageUsageFlags = initialImageUsageFlags;
            for (const auto &newImageGir: newImageGirs) {
                if (!newImageGir->isTransferDestinationDisabled()) {
                    imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                }
                if (!newImageGir->isTransferSourceDisabled()) {
                    imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                }

                imageList.push_back(
                        std::make_shared<VulkanImage>(VulkanImage::CreationInput{
                                newImageGir->getName(),
                                newImageGir->getUid(),
                                VK_IMAGE_TYPE_2D,
                                getVkFormatFromImageGir(newImageGir),
                                getVkExtentFromImageGir(newImageGir),
                                getNumberOfMipLevelsFromImageGir(newImageGir),
                                getNumberOfArrayLevelsFromImageGir(newImageGir),
                                getSampleCountFromImageGir(newImageGir),
                                getImageTilingFromImageGir(newImageGir),
                                imageUsageFlags,
                                // TODO - change this so it's not hardcoded to use graphics queue
                                {device.getGraphicsQueueFamilyIndex()},
                                getInitialImageLayoutFromImageGir(newImageGir),
                                VMA_MEMORY_USAGE_AUTO, // TODO - add configurability for this as needed
                                allocator
                        })
                );
            }
        }

        static VkShaderStageFlags getShaderStageFlagsFromShaderConstantGir(
                const std::shared_ptr<gir::ShaderConstantIR> &shaderConstantGir) {
            // TODO - represent shader constant stage usages? pretty simple to throw in a simple enum converter
            return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // just hardcoding this for now
        }

        void allocateShaderConstantGirs(
                const std::vector<std::shared_ptr<gir::ShaderConstantIR> > &shaderConstantGirs) {
            // aite...
            for (const auto &shaderConstantGir: shaderConstantGirs) {
                shaderConstants.push_back(std::make_shared<VulkanPushConstant>(VulkanPushConstant::CreationInput{
                        shaderConstantGir->getName(),
                        shaderConstantGir->getUid(),
                        getShaderStageFlagsFromShaderConstantGir(shaderConstantGir),
                        shaderConstantGir->getByteOffset(),
                        shaderConstantGir->getSizeInBytes()
                }));
            }
        }
    };
}
