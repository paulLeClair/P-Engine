#pragma once

namespace pEngine::girEngine::scene::graph::renderPass {

/**
* The attachment load operation is the action taken by the graphics backend API
* before beginning a given render pass - it controls what is
* done with the image data *before* rendering
*/
    enum class AttachmentLoadOperation {
        /**
        * This specifies that the pre-existing image data when rendering
        * begins is irrelevant; the initial values are ignored and may
        * be changed
        */
        DONT_CARE,
        /**
        * This specifies that the pre-existing image data is to be kept
        * when rendering begins; no values will be changed.
        */
        LOAD,
        /**
        * This specifies that pre-existing image data is to be set to some
        * specified clear value before rendering begins - initial values will
        * be overwritten
        */
        CLEAR
    };

}
