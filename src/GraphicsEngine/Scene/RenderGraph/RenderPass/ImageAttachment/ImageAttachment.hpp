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
 * Note - we'll probably have to add something for specifying what shader name
 * to look for;
 * \n\n
 * Maybe you should be able to specify either the binding slots themselves
 * or the name of the shader variable? Not really sure - gonna have to learn
 * as I go when it comes to the finer details of shader bindings.
 * \n\n
 * The challenge would be making it platform-agnostic, although I'm
 * not sure that the OpenGL/Vulkan and Microsoft platforms are that
 * different, since I think they just use HLSL which is fairly
 * similar and can be compiled to SPIR-V anyway... At least I'd hope
 * they can't be that different.
 *
*/
    struct ImageAttachment {
//        struct CreationInput {
//            std::shared_ptr<Image> image;
//            AttachmentState imageState;
//            AttachmentLoadOperation loadOperation;
//            AttachmentStoreOperation storeOperation;
//            ClearValue clearValue;
//            MultisampleResolveOperation resolveOperation = MultisampleResolveOperation::NO_OP;
//            AttachmentState resolveState = AttachmentState::UNDEFINED;
//        };
//
//        explicit ImageAttachment(const CreationInput &creationInput)
//                : image(creationInput.image),
//                  imageState(creationInput.imageState),
//                  loadOperation(creationInput.loadOperation),
//                  storeOperation(creationInput.storeOperation),
//                  clearValue(creationInput.clearValue),
//                  resolveOperation(creationInput.resolveOperation),
//                  resolveState(creationInput.resolveState) {
//
//        }

        ImageAttachment() = default;

        /**
        * This is a handle to the scene image that is being attached to a render pass.
        */
        std::shared_ptr<Image> image = nullptr;

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
        * The clear value is used if the specified load operation is set to CLEAR
        */
        ClearValue clearValue = {};

    };

} // scene
