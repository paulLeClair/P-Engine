#pragma once

#include "TexelTypes.hpp"

class TexelFormat {
public:

    // TODO - determine whether it's better to do 1 bool - 1 type, or if we should break it up by category
    virtual bool is_R8G8B8A8_SRGB_Format() {
        return false;
    }

    [[nodiscard]] virtual unsigned int getTexelSizeInBytes() const = 0;

    [[nodiscard]] virtual TexelTypeToken getTexelTypeToken() const = 0;
};