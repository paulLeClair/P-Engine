//
// Created by paull on 2023-04-27.
//

#pragma once


#include <memory>
#include "../SceneResource.hpp"

class Sampler : public SceneResource {
public:
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

    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

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

    explicit Sampler(const CreationInput &creationInput) :
            parentScene(creationInput.parentScene),
            name(creationInput.name),
            uniqueIdentifier(creationInput.uniqueIdentifier),
            mipLevelOfDetailBias(creationInput.mipLevelOfDetailBias),
            maxAnisotropy(creationInput.maxAnisotropy),
            magnificationFilterType(
                    creationInput.magnificationFilterType),
            minificationFilterType(creationInput.minificationFilterType),
            samplerMipmapMode(creationInput.samplerMipmapMode),
            addressMode(creationInput.addressMode),
            pcfCompareOperation(creationInput.pcfCompareOperation),
            samplerAnisotropyEnabledFlag(
                    creationInput.isSamplerAnisotropyEnabled),
            minimumLod(creationInput.minimumLod),
            maximumLod(creationInput.maximumLod),
            samplerUnnormalizedCoordinatesFlag(
                    creationInput.isSamplerUsingUnnormalizedCoordinates),
            samplerPercentageCloserFilteringEnabledFlag(
                    creationInput.isSamplerCompareEnabled) {

    }

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }

    UpdateResult update() override {
        // TODO - determine whether Samplers need to be update-able and change this if so
        return UpdateResult::FAILURE;
    }

    [[nodiscard]] MipLevelOfDetailBias getMipLevelOfDetailBias() const {
        return mipLevelOfDetailBias;
    }

    [[nodiscard]] MaxAnisotropy getMaxAnisotropy() const {
        return maxAnisotropy;
    }

    [[nodiscard]] MagnificationFilterType getMagnificationFilterType() const {
        return magnificationFilterType;
    }

    [[nodiscard]] MinificationFilterType getMinificationFilterType() const {
        return minificationFilterType;
    }

    [[nodiscard]] SamplerMipmapMode getSamplerMipmapMode() const {
        return samplerMipmapMode;
    }

    [[nodiscard]] OutOfBoundsTexelCoordinateAddressMode getAddressMode() const {
        return addressMode;
    }

    [[nodiscard]] PercentageCloserFilteringCompareOperation getPcfCompareOperation() const {
        return pcfCompareOperation;
    }

    [[nodiscard]] bool isSamplerAnisotropyEnabled() const {
        return samplerAnisotropyEnabledFlag;
    }

    [[nodiscard]] float getMinimumLod() const {
        return minimumLod;
    }

    [[nodiscard]] float getMaximumLod() const {
        return maximumLod;
    }

    [[nodiscard]] bool isSamplerUsingUnnormalizedCoordinates() const {
        return samplerUnnormalizedCoordinatesFlag;
    }

    [[nodiscard]] bool isSamplerPCFCompareEnabled() const {
        return samplerPercentageCloserFilteringEnabledFlag;
    }


private:
    std::shared_ptr<Scene> parentScene;

    std::string name;
    PUtilities::UniqueIdentifier uniqueIdentifier;


    MipLevelOfDetailBias mipLevelOfDetailBias;
    MaxAnisotropy maxAnisotropy;

    MagnificationFilterType magnificationFilterType;
    MinificationFilterType minificationFilterType;

    SamplerMipmapMode samplerMipmapMode;
    OutOfBoundsTexelCoordinateAddressMode addressMode;

    bool samplerAnisotropyEnabledFlag;
    bool samplerUnnormalizedCoordinatesFlag;
    bool samplerPercentageCloserFilteringEnabledFlag;

    float minimumLod;
    float maximumLod;

    PercentageCloserFilteringCompareOperation pcfCompareOperation;

};
