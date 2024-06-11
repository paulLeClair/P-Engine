#pragma once

#include "../../../../../lib/glm/ext/vector_int4_sized.hpp"

namespace pEngine::girEngine::scene {

    /**
     * RETURNING TO THIS: I'm not sure what I ended up doing for image formats...
     * I think all images use this texel format currently tho so that's fine.
     *
     * Again this is just kinda a bunch of busy work I think - we're really just
     * translating a vulkan-specific format into a new form.
     *
     * For now I'll just add formats as needed
     */


//    using R8G8B8A8_sRGB_Texel = glm::i8vec4;

/**
 * Each texel type token should map to one texel type(?)
 *
 * Another option: somehow do this with shallow inheritance instead
 * of a map?
 */
    enum class TexelFormat {
        R8G8B8A8_SRGB,
        UNKNOWN
    };
}
