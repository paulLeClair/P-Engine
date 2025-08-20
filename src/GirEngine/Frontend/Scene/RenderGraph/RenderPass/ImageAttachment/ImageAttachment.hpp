#pragma once

#include <memory>

#include "../../../SceneResources/Image/Image.hpp"

#include "AttachmentLoadOperation/AttachmentLoadOperation.hpp"
#include "AttachmentStoreOperation/AttachmentStoreOperation.hpp"
#include "ClearValue/ClearValue.hpp"
#include "MultisampleResolveOperation/MultisampleResolveOperation.hpp"
#include "AttachmentState/AttachmentState.hpp"

namespace pEngine::girEngine::scene::graph::renderPass {
    /**
    * Rendering attachments are specifications of how particular image resources are tied into
    * a render pass for some purpose during rendering.
    *
    * I think it makes the most sense to borrow the terminology of "Attachments" - they are
    * particular resource relationships that a render pass can have where the resources
    * are assigned to a particular slot, which can be one of these:
    * \n\n
    * - Color Input/Output Attachment\n
    * - Multisample Resolve Input/Output Attachment\n
    * - Depth Input/Output Attachment\n
    * - Stencil Input/Output Attachment\n\n
     *
     * TODO -> re-evaluate all this ancient crud; a lot of it can be excised and then re-added later
    */
    struct ImageAttachment {
        /**
        * This is a handle to the scene image that is being attached to a render pass.
        * NOTE: this whole "pass raw pointers" design is easily broken by objects going out of scope,
        * and so it might be the superior choice to just store the ID of the resource
        */
        UniqueIdentifier imageIdentifier;

        ResourceFormat imageFormat = ResourceFormat::UNDEFINED;

        /**
        * The state that the image will be in *during* rendering;
        */
        AttachmentState imageState = AttachmentState::UNDEFINED;

        /**
        * The clear value is used if the specified load operation is set to CLEAR
        */
        ClearValue clearValue = {};

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
    };
} // scene
