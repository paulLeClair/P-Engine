#pragma once

namespace pEngine::girEngine::scene::graph::renderPass {

/**
* The attachment store operation is the action taken by the graphics backend API
* after rendering has completed in a given render pass - it controls what is
* done with the image data *after* rendering
*/
    enum class AttachmentStoreOperation {
        /**
        * This corresponds to the Vulkan-style DONT_CARE option for
        * attachment store operations, where you are
        * signalling that the attachment data is irrelevant to you
        * after the current render pass has used this.
        *
        * Do not use this setting if the image data is to be used in subsequent passes!
        */
        DONT_CARE,
        /**
        * This option specifies that the values (eg pixel color) that are computed
        * as part of rendering are stored in memory.
        */
        STORE
    };

}
