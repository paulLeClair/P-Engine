//
// Created by paull on 2023-10-18.
//

#pragma once

#include "AttachmentState/AttachmentState.hpp"
#include "MultisampleResolveOperation/MultisampleResolveOperation.hpp"
#include "AttachmentLoadOperation/AttachmentLoadOperation.hpp"
#include "AttachmentStoreOperation/AttachmentStoreOperation.hpp"
#include "ClearValue/ClearValue.hpp"

// taken right from scene::renderPass for now; specialize this in the future if needed

namespace pEngine::girEngine::gir::renderPass {

    /**
     * Since I think the idea of having render passes maintain the attachment information
     * on their own is more easily done if we have it match up on the GIR side of things,
     * I'll give this a shot. This basically matches up with the scene render pass attachment info,
     * since it all is being tailored towards vulkan for the time being.
     */
    struct ImageAttachment {

        /**
         * This is the particular image being attached to the RenderPassIR object
         * that this attachment belongs to.
         */
        std::shared_ptr<ImageIR> attachedImage = nullptr;

        /**
        * The state that the image will be in *during* rendering;
        * NOTE - I'm not sure (at time of writing) whether you have to transition the image
        * into the proper state before rendering can begin; it would be doable to insert
        * logic to do that automatically though if need be.
        */
        AttachmentState imageState = AttachmentState::UNDEFINED;

        /**
        * This controls whether the multisample resolve operation is executed after rendering,
        * and if it is executed, what the resolve operation should be.
        */
        MultisampleResolveOperation resolveOperation = MultisampleResolveOperation::NO_OP;

        /**
        * This is the state that the image is expected to be in during resolve operation execution;
        * If you leave it as undefined it will be set to be the same as the image layout that was specified.
        */
        AttachmentState resolveState = AttachmentState::UNDEFINED;

        /**
        * This controls what is done with the attachment image's contents before rendering.
        */
        AttachmentLoadOperation loadOperation = AttachmentLoadOperation::DONT_CARE;

        /**
        * This controls what is done with the attachment image's contents after rendering.
        */
        AttachmentStoreOperation storeOperation = AttachmentStoreOperation::DONT_CARE;

        /**
        * The clear value is used to set all the attached image's pixels if the specified load operation is set to CLEAR
        */
        ClearValue clearValue = {};
    };

} // gir
