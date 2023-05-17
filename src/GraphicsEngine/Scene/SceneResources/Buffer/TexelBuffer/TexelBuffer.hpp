//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../Buffer.hpp"

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
class TexelBuffer : public Buffer {
public:
    enum class UsageType {
        UNIFORM,
        STORAGE
    };

    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;

        UsageType usageType = UsageType::UNIFORM;

        unsigned char *initialDataPointer = nullptr;
        unsigned long initialDataSizeInBytes = 0;

        // TODO - store information about the format of the data

        std::shared_ptr<Sampler> sampler = nullptr;
    };

    explicit TexelBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
            creationInput.parentScene,
            creationInput.name,
            creationInput.uniqueIdentifier,
            creationInput.updateCallback
    }), usageType(creationInput.usageType) {
        rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
                creationInput.name,
                creationInput.uniqueIdentifier,
                creationInput.initialDataPointer,
                creationInput.initialDataSizeInBytes
        });

        sampler = creationInput.sampler;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override {
        return rawDataContainer->getRawDataSizeInBytes();
    }

    [[nodiscard]] UsageType getUsageType() const {
        return usageType;
    }

    [[nodiscard]] const std::shared_ptr<RawDataContainer> &getRawDataContainer() const {
        return rawDataContainer;
    }

    [[nodiscard]] const std::shared_ptr<Sampler> &getSampler() const {
        return sampler;
    }

private:
    UsageType usageType;

    std::shared_ptr<RawDataContainer> rawDataContainer;

    std::shared_ptr<Sampler> sampler;
};
