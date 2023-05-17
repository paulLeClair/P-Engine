//
// Created by paull on 2022-06-19.
//

#include "PBackend.hpp"

#include "../ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.hpp"
#include "VulkanTexture/VulkanTexture.hpp"
#include "../../Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp"
#include "../../Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.hpp"

#include "VulkanRenderable/VulkanVertexList/VulkanVertexList.hpp"
#include "VulkanRenderable/VulkanIndexedMesh/VulkanIndexedMesh.hpp"

namespace PGraphics {

    void PBackend::bakeRenderData() {
        try {
            bakeSceneResources();
            bakeShaderModules();
            bakeDescriptorSetAllocators();
            bakeSceneViews();
            bakeSceneRenderGraph();
        }
        catch (std::exception &exception) {
            // TODO - log!
            throw std::runtime_error("Error during PBackend::bakeRenderData: unable to bake render data!");
        }
    }

    void PBackend::bakeSceneResources() {
        /**
         * New idea: instead of this "first bake primitives then bake composites", it makes a lot more sense
         * and is a lot cleaner (I hope) to instead have every composite resource be responsible for its own
         * primitive resources; they should act as containers for their subresources anyway
         *
         * To maintain homogeneity in the Backend's data, we still will have to amalgamate them by type; this is
         * where it gets a bit more complicated I guess.
         *
         * The other thing is that we need to be able to have composite resources potentially reference the same primitives;
         * in this case, probably just passing handles to actual already-created primitives (non-baked) (or maybe even their UIDs)
         * that will be linked up (whatever that involves) at bake time
         *
         * I think it makes sense to do it like this: each composite resource will simply maintain a list of other high-level resources
         * that it uses - we can just actually bake the resource the first time it's used and then from then on any other composite resources
         * that may use it will able to be to just hook it in from there. That would also make baking scene resources simpler,
         * since we can just iterate over resources in any order and as long as we properly track which resources have been baked it should
         * be fine.
         *
         * The bake process could then be (at a high level anyway):
         *
         *  for all scene resources:
         *      bake particular scene resource - this should be done through the high-level SceneResource::bake() interface function
         *
         * Then as long as each scene resource is able to check which resources have been baked so far, that should be that...
         * granted, for buffers in particular (which have to be split up and suballocated by usage) we have a multipart baking process
         * where we're first gathering up suballocations and then actually setting up the VulkanBuffers to be suballocated afterwards,
         * so maybe we'll have to add in a step after the above loop where we actually suballocate things
         *
         * It might be possible to encapsulate the post-bake process somehow, but for now having a simple and clear "suballocateBakedBuffers()"
         * type thing might be a bit better.
         *
         * ACTUALLY - I think the best way might be to just keep the baking logic in the backend (as opposed to having each resource have an interface function)
         * and then have a global map of "AlreadyBakedResources" by UID, and then we just go through in order and
         * have the bakeBuffer() function maybe use a separate map that stores the VulkanBufferSuballocations or whatever
         * so that we can then use the existing code for separating Scene::Buffers by usage and then suballocating a
         * single global buffer for each usage type
         *
         * Trying this: renaming the baked resources struct to "BakeOutput" and then having a function "finalizeSceneResourcesBakeOutput()"
         * or something like that
         *
         * Hmmmmm actually maybe we don't even need this "BakeOutput" struct actually.......... I may just be a dummy and rewrote everything
         * for no reason
         */
        bakeSceneTextures();
        bakeSceneImages();
        bakeSceneBuffers();
        bakeSceneMaterials();
        bakeSceneModels();
        bakeSceneShaderConstants();
    }

    void PBackend::bakeSceneImages() {
        for (const auto &image: scene->getImages()) {
            bakeImage(image);
        }
    }

    void PBackend::bakeSceneShaderConstants() {
        for (const auto &sceneConstant: scene->getShaderConstants()) {
            bakeShaderConstant(sceneConstant);
        }
    }

    void PBackend::bakeShaderConstant(const std::shared_ptr<ShaderConstant> &shaderConstant) {
        if (vulkanPushConstantUids.count(shaderConstant->getUniqueIdentifier())) {
            // TODO - log (optionally) that we already baked this constant
            return;
        }

        // TODO - bake a VulkanPushConstant (or maybe VulkanPushConstantRange?) based off this
        // I'm still a little unsure about the best way to abstract VkPushConstants anyway, maybe a simple
        // mapping of "Scene::ShaderConstant with some data -> VkPushConstantRange whose size is equal to the size of the data"
        VulkanPushConstant::CreationInput vulkanPushConstantCreationInput = {
                shaderConstant->getName(),
                shaderConstant->getUniqueIdentifier(),
                getVkShaderStageFlagsFromShaderConstantShaderStages(shaderConstant->getShaderStages()),
                shaderConstant->getOffset(),
                shaderConstant->getSize()
        };

        vulkanPushConstants.push_back(std::make_shared<VulkanPushConstant>(vulkanPushConstantCreationInput));
        vulkanPushConstantUids[shaderConstant->getUniqueIdentifier()] = vulkanPushConstants.size() - 1;
    }

    VkShaderStageFlags
    PBackend::getVkShaderStageFlagsFromShaderConstantShaderStages(
            const std::vector<ShaderConstant::ShaderStage> &shaderConstantShaderStages) {
        VkShaderStageFlags shaderStageFlags = 0;

        static const std::unordered_map<ShaderConstant::ShaderStage, VkShaderStageFlagBits> shaderConstantStageToVkShaderStageFlagBits = {
                {ShaderConstant::ShaderStage::VERTEX,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT},
                {ShaderConstant::ShaderStage::TESSELLATION_CONTROL,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
                {ShaderConstant::ShaderStage::TESSELLATION_EVALUATION,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
                {ShaderConstant::ShaderStage::GEOMETRY,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT},
                {ShaderConstant::ShaderStage::FRAGMENT,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT},
                // TODO - implement missing shader stage flags (not sure how to handle the ones that require extensions)
                {ShaderConstant::ShaderStage::ALL_GRAPHICS,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS}
        };

        for (const auto &shaderStage: shaderConstantShaderStages) {
            shaderStageFlags |= shaderConstantStageToVkShaderStageFlagBits.at(shaderStage);
        }

        return shaderStageFlags;
    }

    void PBackend::bakeImage(const std::shared_ptr<Image> &image) {
        if (vulkanImageUids.count(image->getUniqueIdentifier())) {
            // TODO - optionally log that we already baked this image
            return;
        }

        // on to images I guess!
        VulkanImage::CreationInput imageCreationInput = {
                image->getName(),
                image->getUniqueIdentifier(),
                VkImageType::VK_IMAGE_TYPE_2D, // TODO - make this configurable
                convertToVulkanImageFormat(*image->getFormat()),
                getVkExtent3DFromImageExtent(image->getImageExtent2D()),
                1, // TODO - add mip stuff
                1, // TODO - add image array stuff
                getVkSampleCountFromNumberOfSamples(image->getNumberOfSamples()),
                VK_IMAGE_TILING_OPTIMAL, // TODO - add a way to control image tiling somehow...? maybe not?
                convertToVkImageUsageFlags(image->getImageUsages()),
                {applicationContext->getGraphicsQueueFamilyIndex()},
                VK_IMAGE_LAYOUT_UNDEFINED, // TODO - make this depend on the image usages somehow...?
                VMA_MEMORY_USAGE_AUTO,
                allocator
        };

        // TODO - write a utility function that does this so we can factor out all the similar logic in the code (cause we do this a million times i think)
        vulkanImages.push_back(std::make_shared<VulkanImage>(imageCreationInput));
        vulkanImageUids[image->getUniqueIdentifier()] = vulkanImages.size() - 1;
    }

    VkFormat PBackend::convertToVulkanImageFormat(TexelFormat &format) {
        if (format.is_R8G8B8A8_SRGB_Format()) {
            // TODO - log and probably throw an error here instead of returning this value
            return VK_FORMAT_R8G8B8A8_SRGB;
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkExtent3D PBackend::getVkExtent3DFromImageExtent(Image::ImageExtent2D extent2D) {
        return {extent2D.x, extent2D.y, 0};
    }

    VkSampleCountFlagBits PBackend::getVkSampleCountFromNumberOfSamples(unsigned int samples) {
        switch (samples) {
            case 2:
                return VK_SAMPLE_COUNT_2_BIT;
            case 4:
                return VK_SAMPLE_COUNT_4_BIT;
            case 8:
                return VK_SAMPLE_COUNT_8_BIT;
            case 16:
                return VK_SAMPLE_COUNT_16_BIT;
            case 32:
                return VK_SAMPLE_COUNT_32_BIT;
            case 64:
                return VK_SAMPLE_COUNT_64_BIT;
            default:
                return VK_SAMPLE_COUNT_1_BIT;
        }
    }

    VkImageUsageFlags PBackend::convertToVkImageUsageFlags(const std::unordered_set<Image::ImageUsage> &imageUsages) {
        static const std::unordered_map<Image::ImageUsage, VkImageUsageFlagBits> IMAGE_USAGE_TO_VK_IMAGE_USAGE_FLAG_BITS = {
                {Image::ImageUsage::TransferDestination,    VK_IMAGE_USAGE_TRANSFER_DST_BIT},
                {Image::ImageUsage::TransferSource,         VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
                {Image::ImageUsage::Storage,                VK_IMAGE_USAGE_STORAGE_BIT},
                {Image::ImageUsage::ColorAttachment,        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
                {Image::ImageUsage::DepthStencilAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
                {Image::ImageUsage::InputAttachment,        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
                {Image::ImageUsage::ShaderSampled,          VK_IMAGE_USAGE_SAMPLED_BIT},
                {Image::ImageUsage::TransientAttachment,    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT}
        };

        VkImageUsageFlags imageUsageFlags = 0;

        for (auto imageUsage: imageUsages) {
            if (IMAGE_USAGE_TO_VK_IMAGE_USAGE_FLAG_BITS.count(imageUsage)) {
                imageUsageFlags |= IMAGE_USAGE_TO_VK_IMAGE_USAGE_FLAG_BITS.at(imageUsage);
            }
        }

        return imageUsageFlags;
    }

    void PBackend::bakeSceneBuffers() {
        std::vector<std::shared_ptr<Buffer>> vertexIndexBuffers = {};
        std::vector<std::shared_ptr<Buffer>> uniformBuffers = {};
        std::vector<std::shared_ptr<Buffer>> texelBuffers = {};
        std::vector<std::shared_ptr<Buffer>> storageBuffers = {};

        groupSceneBuffersByBufferUsage(vertexIndexBuffers, uniformBuffers, texelBuffers, storageBuffers);

        vertexIndexBufferSuballocator->suballocateBuffers(vertexIndexBuffers);
        uniformBufferSuballocator->suballocateBuffers(uniformBuffers);
        texelBufferSuballocator->suballocateBuffers(texelBuffers);
        storageBufferSuballocator->suballocateBuffers(storageBuffers);
    }

    void PBackend::groupSceneBuffersByBufferUsage(std::vector<std::shared_ptr<Buffer>> &vertexIndexBuffers,
                                                  std::vector<std::shared_ptr<Buffer>> &uniformBuffers,
                                                  std::vector<std::shared_ptr<Buffer>> &texelBuffers,
                                                  std::vector<std::shared_ptr<Buffer>> &storageBuffers) const {
        for (auto &buffer: scene->getBuffers()) {

            // basically, depending on the usage flag we give it to the appropriate buffer suballocator
            if (buffer->isGeometryBuffer()) {
                vertexIndexBuffers.push_back(buffer);
            } else if (buffer->isUniformBuffer()) {
                uniformBuffers.push_back(buffer);
            } else if (buffer->isTexelBuffer()) {
                texelBuffers.push_back(buffer);
            } else if (buffer->isStorageBuffer()) {
                storageBuffers.push_back(buffer);
            }
        }
    }

    void PBackend::bakeSceneTextures() {
        for (const auto &texture: scene->getTextures()) {
            bakeTexture(texture);
        }
    }

    void PBackend::bakeTexture(const std::shared_ptr<Texture> &sharedPtr) {
        if (vulkanTextureUids.count(sharedPtr->getUniqueIdentifier())) {
            // TODO - optionally log that the texture has already been baked
            return;
        }

        VulkanTexture::CreationInput vulkanTextureCreationInput = {
                sharedPtr->getName(),
                sharedPtr->getUniqueIdentifier(),
                sharedPtr->getResourceConfiguration(),
                applicationContext->getLogicalDevice(),
                bakeVulkanSamplerCreateInfoFromTexture(sharedPtr),
                bakeVulkanImageCreateInfoFromTexture(sharedPtr)
        };

        vulkanTextures.push_back(std::make_shared<VulkanTexture>(vulkanTextureCreationInput));
        vulkanTextureUids[sharedPtr->getUniqueIdentifier()] = vulkanTextures.size() - 1;
    }

    VkSamplerCreateInfo PBackend::bakeVulkanSamplerCreateInfoFromTexture(const std::shared_ptr<Texture> &texture) {
        return {
                VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                nullptr,
                0,
                getTextureMinificationFilter(texture),
                getTextureMagnificationFilter(texture),
                getSamplerMipmapMode(texture),
                getSamplerAddressMode(texture),
                getSamplerAddressMode(texture),
                getSamplerAddressMode(texture),
                getSamplerMipLodBias(texture),
                getSamplerAnisotropyEnableFlag(texture),
                getSamplerMaxAnisotropy(texture),
                getSamplerCompareEnableFlag(texture),
                getSamplerCompareOp(texture),
                getSamplerMinLod(texture),
                getSamplerMaxLod(texture),
                getSamplerBorderColor(texture),
                getSamplerUnnormalizedCoordinatesFlag(texture)
        };
    }

    VkFilter PBackend::getTextureMinificationFilter(const std::shared_ptr<Texture> &texture) {
        if (texture->getMinificationFilterType() == Sampler::MinificationFilterType::NEAREST_TEXEL) {
            return VK_FILTER_NEAREST;
        }
        return VK_FILTER_LINEAR;
    }

    VkFilter PBackend::getTextureMagnificationFilter(const std::shared_ptr<Texture> &texture) {
        if (texture->getMagnificationFilterType() == Sampler::MagnificationFilterType::NEAREST_TEXEL) {
            return VK_FILTER_NEAREST;
        }
        return VK_FILTER_LINEAR;
    }

    VkSamplerMipmapMode PBackend::getSamplerMipmapMode(const std::shared_ptr<Texture> &texture) {
        switch (texture->getSamplerMipmapMode()) {
            case (Sampler::SamplerMipmapMode::LINEAR_BLEND): {
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            }
            case (Sampler::SamplerMipmapMode::ROUND_TO_NEAREST_INTEGER): {
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            }
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    VkSamplerAddressMode PBackend::getSamplerAddressMode(const std::shared_ptr<Texture> &texture) {
        switch (texture->getAddressMode()) {
            case (Sampler::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_BORDER): {
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            }
            case (Sampler::OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_EDGE): {
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            }
            case (Sampler::OutOfBoundsTexelCoordinateAddressMode::MIRRORED_REPEAT): {
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            }
            case (Sampler::OutOfBoundsTexelCoordinateAddressMode::MIRROR_CLAMP_TO_EDGE): {
                return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
            }
            case (Sampler::OutOfBoundsTexelCoordinateAddressMode::REPEAT): {
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
            default:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    float PBackend::getSamplerMipLodBias(const std::shared_ptr<Texture> &texture) {
        return texture->getMipLevelOfDetailBias();
    }

    VkBool32 PBackend::getSamplerAnisotropyEnableFlag(const std::shared_ptr<Texture> &texture) {
        return (texture->getSampler()->isSamplerAnisotropyEnabled()) ? VK_TRUE : VK_FALSE;
    }

    float PBackend::getSamplerMaxAnisotropy(const std::shared_ptr<Texture> &texture) {
        return texture->getMaxAnisotropy();
    }

    VkBool32 PBackend::getSamplerCompareEnableFlag(const std::shared_ptr<Texture> &texture) {
        return texture->getSampler()->isSamplerPCFCompareEnabled();
    }

    VkCompareOp PBackend::getSamplerCompareOp(const std::shared_ptr<Texture> &texture) {
        switch (texture->getPcfCompareOperation()) {
            case (Sampler::PercentageCloserFilteringCompareOperation::ALWAYS): {
                return VK_COMPARE_OP_ALWAYS;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::EQUALS): {
                return VK_COMPARE_OP_EQUAL;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::NOT_EQUALS): {
                return VK_COMPARE_OP_NOT_EQUAL;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::GREATER_THAN): {
                return VK_COMPARE_OP_GREATER;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::GREATER_THAN_EQUALS): {
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::LESS_THAN): {
                return VK_COMPARE_OP_LESS;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::LESS_THAN_EQUALS): {
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            }
            case (Sampler::PercentageCloserFilteringCompareOperation::NEVER): {
                return VK_COMPARE_OP_NEVER;
            }
            default:
                return VK_COMPARE_OP_NEVER;
        }
    }

    float PBackend::getSamplerMinLod(const std::shared_ptr<Texture> &texture) {
        return texture->getSampler()->getMinimumLod();
    }

    float PBackend::getSamplerMaxLod(const std::shared_ptr<Texture> &texture) {
        return texture->getSampler()->getMaximumLod();
    }


    VkBorderColor PBackend::getSamplerBorderColor(const std::shared_ptr<Texture> &texture) {
        // TODO - make this configurable
        return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    }

    VkBool32 PBackend::getSamplerUnnormalizedCoordinatesFlag(const std::shared_ptr<Texture> &texture) {
        return texture->getSampler()->isSamplerUsingUnnormalizedCoordinates();
    }

    VkImageCreateInfo PBackend::bakeVulkanImageCreateInfoFromTexture(const std::shared_ptr<Texture> &texture) {
        if (texture->getResourceConfiguration() == Texture::ResourceConfiguration::SAMPLER_ONLY) {
            // TODO - log!
            throw std::runtime_error(
                    "Error in PBackend::bakeVulkanImageCreateInfoFromTexture() - attempting to bake vulkan image in sampler-only texture!");
        }

        // TODO
        return VkImageCreateInfo{
                VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                nullptr,
                0,
                // TODO
        };
    }

    void PBackend::bakeSceneModels() {
        for (const auto &model: scene->getModels()) {
            bakeModel(model);
        }
    }

    /**
     *  NOW - baking these models...
     *
     * Since models currently are purely composite resources, it may be kind of easy to do this I think
     *
     * The only thing is that I don't know exactly if we're going to want to make a Vulkan-facing abstraction that
     * corresponds to one of these Scene::Models.
     *
     * For now maybe I'll avoid it, since really Models are just meant to be a nice way to logically group geometry
     * that's drawn/updated together (which lines up with the traditional notion of a model fairly well I hope),
     * and really the backend just needs to be concerned with drawing batches of pre-baked geometry so we can try
     * stripping away that aspect of it.
     *
     * However I suspect in the future we'll need to add more structure here because there's a lot of stuff going on
     * with managing scene geometry data (eg animations, etc) and I can't account for it all at once right off the bat.
     * Hopefully it will pan out that I won't need to go too crazy, but we can modify the model bake process later
     * if we need to add more stuff on top.
     *
     * @param model the model to be baked
     */
    void PBackend::bakeModel(const std::shared_ptr<Model> &model) {
        for (const auto &image: model->getImages()) {
            bakeImage(image);
        }

        std::vector<std::shared_ptr<Buffer>> modelBuffers = model->getBuffers();
        bakeListOfBuffers(modelBuffers);

        for (const auto &shaderConstant: model->getShaderConstants()) {
            bakeShaderConstant(shaderConstant);
        }

        for (const auto &material: model->getMaterials()) {
            bakeMaterial(material);
        }

        for (const auto &texture: model->getTextures()) {
            bakeTexture(texture);
        }

        for (const auto &renderable: model->getRenderables()) {
            bakeRenderable(renderable);
        }
    }

    void PBackend::bakeListOfBuffers(std::vector<std::shared_ptr<Buffer>> &buffers) {
        for (const auto &buffer: buffers) {
            if (buffer->isGeometryBuffer()) { // TODO - split this into VertexBuffer and IndexBuffer
                vertexIndexBufferSuballocator->suballocateBuffer(buffer);
            }

            if (buffer->isUniformBuffer()) {
                uniformBufferSuballocator->suballocateBuffer(buffer);
            }

            if (buffer->isStorageBuffer()) {
                storageBufferSuballocator->suballocateBuffer(buffer);
            }

            if (buffer->isTexelBuffer()) {
                texelBufferSuballocator->suballocateBuffer(buffer);
            }
        }
    }

    void PBackend::bakeSceneMaterials() {
        for (const auto &material: scene->getMaterials()) {
            bakeMaterial(material);
        }
    }

    void PBackend::bakeMaterial(const std::shared_ptr<Material> &material) {
        // Similar to models, I'm not entirely sure if we'll want to add anything extra on top of this, but for now
        // materials will just behave as pure composite resources

        auto materialBuffers = material->getBuffers();
        bakeListOfBuffers(materialBuffers);

        for (const auto &image: material->getImages()) { // TODO - refactor out all these loops into "bakeListOf*" like above
            bakeImage(image);
        }

        for (const auto &texture: material->getTextures()) {
            bakeTexture(texture);
        }

        for (const auto &shaderConstant: material->getShaderConstants()) {
            bakeShaderConstant(shaderConstant);
        }
    }

    void PBackend::bakeRenderable(const std::shared_ptr<Renderable> &renderable) {
        auto renderableType = renderable->getType();

        switch (renderableType) {
            case (Renderable::RenderableType::TriangleMesh): {
                bakeTriangleMesh(std::dynamic_pointer_cast<TriangleMesh>(renderable));
                break;
            }
            case (Renderable::RenderableType::IndexedTriangleMesh): {
                bakeIndexedTriangleMesh(std::dynamic_pointer_cast<IndexedTriangleMesh>(renderable));
                break;
            }
            default: {
                throw std::runtime_error("Error in PBackend::bakeRenderable() - Unsupported renderable type!");
            }
        }
    }

    void PBackend::bakeTriangleMesh(const std::shared_ptr<TriangleMesh> &triangleMeshToBake) {
        // TODO - bake triangle meshes in particular
        // I believe each of these will just get baked into a VulkanVertexList...?
        if (vulkanRenderableUids.count(triangleMeshToBake->getUniqueIdentifier())) {
            return;
        }

        // in this case I believe we basically just have to suballocate a buffer for the vertex data...?
        // TODO - rewrite how this works

        vulkanRenderableUids.insert(triangleMeshToBake->getUniqueIdentifier());
    }

    void PBackend::bakeIndexedTriangleMesh(const std::shared_ptr<IndexedTriangleMesh> &indexedTriangleMeshToBake) {
        // TODO - bake indexed triangle meshes in particular
        if (vulkanRenderableUids.count(indexedTriangleMeshToBake->getUniqueIdentifier())) {
            return;
        }

        // TODO - rewrite how this works

        vulkanRenderableUids.insert(indexedTriangleMeshToBake->getUniqueIdentifier());
    }

    void PBackend::bakeShaderModules() {
        // TODO
        // note for implementation: here we should be also baking a set of descriptor set layouts for each of the possible
        // slots (except global resources, which all shaders must use the same global layout)

        // NOTE - this will probably to be done per render pass, ie this function might have to be moved to be internal to the scope
        // of bakeSceneRenderGraph() (and bakeRenderPass() in particular)
    }

    void PBackend::bakeDescriptorSetAllocators() {
        // based on the descriptor set layouts reflected from the shader modules, here we would bake a descriptor set
        // allocator for each descriptor set slot, which will then be used to allocate descriptor sets as needed for each
        // set

        // NOTE - this will probably to be done per render pass, ie this function might have to be moved to be internal to the scope
        // of bakeSceneRenderGraph() (and bakeRenderPass() in particular)
    }

    void PBackend::bakeSceneViews() {
        // TODO - figure out how to convert the Scene camera abstraction into the relevant Vulkan structures
    }

    void PBackend::bakeSceneRenderGraph() {
        // this is a meaty one and involves hooking together all the baked resources and shader modules with
        // freshly-baked render passes (built off the Scene's render passes of course)

    }

    [[maybe_unused]] VkBufferUsageFlags PBackend::convertToVkBufferUsageFlags(unsigned int flags) {
        // here we convert the high-level Scene::Buffer flags to VkBufferUsageFlags

        return 0;
    }

    Backend::DrawFrameResult PBackend::drawFrame() {
        if (!hasBeenBaked) {
            // TODO - logger!
            return Backend::DrawFrameResult::FAILURE; // TODO - make seperate enum value for this case
        }

        try {
            // if it has been baked, we want to commit any per-frame updates
            if (commitPerFrameUpdates((const PScene::PerFrameUpdates &) scene->getPerFrameUpdates()) !=
                CommitPerFrameUpdatesResult::SUCCESS) {
                // TODO - handle this somehow lol
            }

            frameExecutionController->executeNextFrame();
            return Backend::DrawFrameResult::SUCCESS;
        }
        catch (std::exception &exception) {
            // TODO - use logger to warn!
            return Backend::DrawFrameResult::FAILURE;
        }
    }

    /**
     * I'm specializing this class to use PScene's subclass of Scene::PerFrameUpdates,
     * but it might be worth it to redo the entire thing into a separate class
     * @param perFrameUpdates
     * @return
     */
    PBackend::CommitPerFrameUpdatesResult
    PBackend::commitPerFrameUpdates(const PScene::PerFrameUpdates &perFrameUpdates) {
        if (perFrameUpdates.updatedBuffers.empty() && perFrameUpdates.updatedImages.empty() &&
            perFrameUpdates.updatedModels.empty()) {
            return PBackend::CommitPerFrameUpdatesResult::SUCCESS;
        }

        for (auto &updatedBufferUid: perFrameUpdates.updatedBuffers) {

            if (!commitBufferUpdate(updatedBufferUid)) {
                // TODO - log here and probably return a particular error code!
                return CommitPerFrameUpdatesResult::FAILURE;
            }
        }

        for (auto &updatedImageUid: perFrameUpdates.updatedImages) {

            if (!commitImageUpdate(updatedImageUid)) {
                // TODO - log here and probably return a particular error code!
                return CommitPerFrameUpdatesResult::FAILURE;
            }
        }

        for (auto &updatedModelUid: perFrameUpdates.updatedModels) {

            if (!commitModelUpdate(updatedModelUid)) {
                // TODO - log here and probably return a particular error code!
                return CommitPerFrameUpdatesResult::FAILURE;
            }
        }

        return PBackend::CommitPerFrameUpdatesResult::SUCCESS;
    }

    bool PBackend::commitBufferUpdate(const PUtilities::UniqueIdentifier &bufferUid) const {
        auto updatedBuffer = scene->getBuffer(bufferUid);


        return true;
    }

    bool PBackend::commitImageUpdate(const PUtilities::UniqueIdentifier &imageUid) const {
        auto updatedImage = scene->getImage(imageUid);

        return true;
    }

    bool PBackend::commitModelUpdate(const PUtilities::UniqueIdentifier &modelUid) const {
        auto updatedModel = scene->getModel(modelUid);

        return true;
    }


}// namespace PGraphics