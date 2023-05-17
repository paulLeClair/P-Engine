//
// Created by paull on 2023-05-15.
//

#pragma once

#include "../TexelFormat.hpp"

class R8G8B8A8_SRGB_TexelFormat : public TexelFormat {
public:
    bool is_R8G8B8A8_SRGB_Format() override {
        return true;
    }

    [[nodiscard]] unsigned int getTexelSizeInBytes() const override {
        return sizeof(R8G8B8A8_sRGB_Texel);
    }

    [[nodiscard]] TexelTypeToken getTexelTypeToken() const override {
        return TexelTypeToken::R8G8B8A8_SRGB;
    }
};