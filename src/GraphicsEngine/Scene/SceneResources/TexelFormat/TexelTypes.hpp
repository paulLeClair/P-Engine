//
// Created by paull on 2023-05-12.
//

#pragma once

// TODO - add more texel types

#include "../../../../lib/glm/ext/vector_int4_sized.hpp"

/**
 * Each texel type token should map to one texel type...?
 */
enum class TexelTypeToken {
    R8G8B8A8_SRGB,
    UNKNOWN
};

using R8G8B8A8_sRGB_Texel = glm::i8vec4;

