//
// Created by paull on 2022-06-28.
//

#pragma once

namespace pEngine::girEngine::backend::vulkan {


    /**
     * Now that we use the whole VulkanBufferSuballocator thing,
     * we probably want to have these classes just store the offset (in the suballocated buffer) and size, plus any
     * additional info needed for drawing the renderable
     *
     * In any case we want to look at the VulkanBufferSuballocator to determine what's needed here
     *
     * Now that I think about it, since any buffers they use will be
     * handled in the VulkanBufferSuballocators, and similarly any other resources will have been baked into samplers/images/etc,
     * unless we need to group everything up for drawing or something like that maybe we don't even need these VulkanRenderables.
     */
    class VulkanRenderable {
    public:
        ~VulkanRenderable() = default;

        virtual bool isVulkanVertexList() {
            return false;
        }

        virtual bool isVulkanIndexedMesh() {
            return false;
        }
    };

}// namespace PGraphics
