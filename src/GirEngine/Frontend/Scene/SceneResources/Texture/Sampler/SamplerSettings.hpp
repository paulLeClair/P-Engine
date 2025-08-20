//
// Created by paull on 2023-04-27.
//

#pragma once


#include <memory>
#include "../../SceneResource.hpp"

namespace pEngine::girEngine::scene {

    struct SamplerSettings {
        using MipLevelOfDetailBias = float;

        using MaxAnisotropy = float;

        enum class MagnificationFilterType {
            NEAREST_TEXEL,
            LINEAR_BLEND
        };


        enum class MinificationFilterType {
            NEAREST_TEXEL,
            LINEAR_BLEND
        };

        enum class SamplerMipmapMode {
            ROUND_TO_NEAREST_INTEGER,
            LINEAR_BLEND
        };

        enum class OutOfBoundsTexelCoordinateAddressMode {
            REPEAT,
            MIRRORED_REPEAT,
            CLAMP_TO_EDGE,
            CLAMP_TO_BORDER,
            MIRROR_CLAMP_TO_EDGE
        };

        enum class PercentageCloserFilteringCompareOperation {
            ALWAYS,
            NEVER,
            GREATER_THAN_EQUALS,
            GREATER_THAN,
            EQUALS,
            NOT_EQUALS,
            LESS_THAN,
            LESS_THAN_EQUALS
        };

        MipLevelOfDetailBias mipLevelOfDetailBias = 0.0f;
        MaxAnisotropy maxAnisotropy = 0.0f;

        MagnificationFilterType magnificationFilterType = MagnificationFilterType::LINEAR_BLEND;
        MinificationFilterType minificationFilterType = MinificationFilterType::LINEAR_BLEND;

        SamplerMipmapMode samplerMipmapMode = SamplerMipmapMode::LINEAR_BLEND;
        OutOfBoundsTexelCoordinateAddressMode addressMode = OutOfBoundsTexelCoordinateAddressMode::CLAMP_TO_EDGE;

        bool isSamplerAnisotropyEnabled = false;
        bool isSamplerUsingUnnormalizedCoordinates = false;
        bool isSamplerPCFEnabled = false;

        float minimumLod = 0.0f;
        float maximumLod = 0.0f;

        PercentageCloserFilteringCompareOperation pcfCompareOperation = PercentageCloserFilteringCompareOperation::NEVER;

    };

}