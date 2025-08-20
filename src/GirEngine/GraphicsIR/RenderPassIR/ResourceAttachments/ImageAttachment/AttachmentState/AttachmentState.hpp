#pragma once

// taken right from scene::renderPass for now; specialize this in the future if needed


namespace pEngine::girEngine::gir::renderPass {
/**
 * This may not have an analogue in APIs that aren't Vulkan (ie DX12)
 * but all of them have some way of transitioning resources from one usage
 * to another I believe (though I may be wrong since I only really know Vulkan)
 * 
 * This should only cover image usages, but we can obtain it for buffer resources
 * in other ways I think.
 * 
 * TODO - maybe move this so that it's internal to an Image? But first look into what
 * the best practices would be here...
*/
    enum class AttachmentState {
        /**
         * This should not be a valid state that an image should be in while it's used
        */
        UNDEFINED,
        /**
         * Preinitialized means that the resource is currently holding user-specified data
         * before any rendering operations have been done
        */
        PREINITIALIZED,
        /**
         * In this state, the image is being used as a color attachment (render target)
        */
        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_STENCIL_READ_ONLY_ATTACHMENT,
        DEPTH_READ_ONLY_ATTACHMENT,
        DEPTH_ATTACHMENT,
        STENCIL_READ_ONLY_ATTACHMENT,
        STENCIL_ATTACHMENT,
        TRANSFER_SOURCE,
        TRANSFER_DESTINATION,
        /**
         * I think this is the state that's meant for images being used as textures
         * or as storage
        */
        SHADER_READ_ONLY,
        /**
         * This state signifies that the image is able to be presented via a
         * windowing system
        */
        PRESENT_MODE,
        SWAPCHAIN_COLOR_ATTACHMENT
    };

} // scene
