//
// Created by paull on 2023-10-18.
//

#pragma once

namespace pEngine::girEngine::gir::renderPass {

    /**
     * This is TODO mostly until we get to the vulkan backend bake;
     * any other info that isn't contained in the buffer itself can be added here
     */
    struct BufferAttachment {
        gir::BufferIR attachedBuffer = {};
    };

} // gir
