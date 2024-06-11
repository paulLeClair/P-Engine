//
// Created by paull on 2023-10-18.
//

#pragma once

namespace pEngine::girEngine::gir::renderPass {


    /**
     * This contains the relevant information for binding a texture
     * to a render pass; currently based mostly off of the Vulkan API's
     * requirements for information about textures/samplers etc.
     */
    struct TextureAttachment {

        using MipLevelOfDetailBias = float;

        using MaxAnisotropy = float;

        /**
         * This denotes a particular shader stage that you're binding the texture to
         */
        enum class ShaderStage {
            VERTEX,
            FRAGMENT
        };

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

        /**
         * From scene::Texture: the image being sampled
         */
        std::shared_ptr<ImageIR> sampledImage;

        std::vector<ShaderStage> shaderStages = {ShaderStage::FRAGMENT};

        MipLevelOfDetailBias mipLevelOfDetailBias;
        MaxAnisotropy maxAnisotropy;

        MagnificationFilterType magnificationFilterType;
        MinificationFilterType minificationFilterType;

        SamplerMipmapMode samplerMipmapMode;

        OutOfBoundsTexelCoordinateAddressMode addressMode;

        PercentageCloserFilteringCompareOperation pcfCompareOperation;

        bool isSamplerAnisotropyEnabled;

        float minimumLod;

        float maximumLod;

        bool isSamplerUsingUnnormalizedCoordinates;

        bool isSamplerCompareEnabled;

    };

} // gir::renderPass
