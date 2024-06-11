//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../Buffer.hpp"
#include "../../formats/TexelFormat/TexelFormat.hpp"
#include "../../Texture/Sampler/SamplerSettings.hpp"

/**
 * Texel buffers are used for image-like data, so there should be significant overlap between these two classes.
 *
 * However, I'm thinking maybe these can kind of behave like VertexBuffers, in that they're templated - maybe instead of
 * being the vertex type it could potentially be the format or something? But I'm not sure, that may be better if it came
 * from the creation input or something - I'm not really sure how we're going to handle the hundreds of possible formats.
 *
 * If it comes down to it I'll implement a big wrapper class that you just supply certain aspects of the format thru enums,
 * eg "R, RG, RGB..." for the number of dimensions per pixel, the number of bits per coordinate, and the specific type info
 * of the pixel values (eg normalized, unnormalized, unsigned, etc)
 *
 * At least if I did it that way it would be able to be shared between these classes and the image.
 *
 * BIG OTHER THING I JUST REMEMBERED - you basically treat these as sampled images in your shaders; this class will need
 * to follow the example of Texture as well
 *
 * Another note: you can have uniform and storage texel buffers; make that a configuration setting
 */
namespace pEngine::girEngine::scene {
    class TexelBuffer : public Buffer {
    public:
        struct CreationInput : public Buffer::CreationInput {
            SamplerSettings samplerSettings;

            TexelFormat texelFormat;
        };

        explicit TexelBuffer(const CreationInput &creationInput)
            : Buffer(creationInput),
              samplerSettings(creationInput.samplerSettings),
              texelFormat(creationInput.texelFormat) {
        }

        [[nodiscard]] const SamplerSettings &getSampler() const {
            return samplerSettings;
        }

        [[nodiscard]] TexelFormat getTexelFormat() const {
            return texelFormat;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
                getName(),
                getUid(),
                gir::GIRSubtype::BUFFER,
                {gir::BufferIR::BufferUsage::TEXEL_BUFFER},
                getRawDataContainer().getRawDataByteArray(),
                getRawDataContainer().getRawDataSizeInBytes()
            });
        }

    private:
        SamplerSettings samplerSettings;

        TexelFormat texelFormat;
    };
}
