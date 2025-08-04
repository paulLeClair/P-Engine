//
// Created by paull on 2023-10-18.
//

#pragma once

#include "../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

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
     *
     * NOTE -> we're using raw pointers to pre-baked IR images that should exist elsewhere;
     * it would be ideal to have a clean simple design that avoids this danger somehow so that's TODO
     */
    struct ImageAttachment {
        /**
         * This is the particular image being attached to the RenderPassIR object
         * that this attachment belongs to.
         *
         * Using a pointer isn't working because the objects they point to will usually go out of scope,
         * and I don't feel like refactoring it around that. So instead we'll just store the UID
         * since that's what we're comparing against anyway.
         */
        util::UniqueIdentifier attachedImage = {};

        resource::FormatIR imageFormat = resource::FormatIR::UNDEFINED;

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
