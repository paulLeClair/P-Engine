//
// Created by paull on 2023-10-05.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass {

    /**
     * This is an attachment of a particular buffer resource to a render pass...
     * For now, I think the buffer itself holds enough information to go off, and then
     * we can come back and extend this when we're doing the next pass over the Vulkan backend.
     */
    struct BufferAttachment {
        std::shared_ptr<Buffer> buffer = nullptr;

    };

} // scene
