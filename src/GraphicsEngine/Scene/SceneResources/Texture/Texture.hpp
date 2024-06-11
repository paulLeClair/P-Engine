//
// Created by paull on 2023-01-08.
//

#pragma once

#include <memory>
#include "../Image/Image.hpp"
#include "Sampler/SamplerSettings.hpp"

namespace pEngine::girEngine::scene {
    /**
     * This corresponds to whatever backend resources are required to create particular texture instances that
     * can be sampled by shaders.
     *
     * This at least supports all the stuff you need to make a sampler in Vulkan!
     */
    class Texture : public Resource {
    public:
        // TODO - figure out how best to control the usages of textures;
        // i'm thinking it might be fine to just specify whether you want it to be
        // a transfer source/dest as well

        struct CreationInput : public Resource::CreationInput {
            SamplerSettings sampler;

            std::shared_ptr<Image> sampledImage;
        };

        explicit Texture(const CreationInput &creationInput) : Resource(creationInput),
                                                               sampledImage(creationInput.sampledImage),
                                                               samplerSettings(creationInput.sampler) {
        }

        ~Texture() override = default;

        [[nodiscard]] SamplerSettings::MipLevelOfDetailBias getMipLevelOfDetailBias() const {
            return samplerSettings.mipLevelOfDetailBias;
        }

        [[nodiscard]] SamplerSettings::MaxAnisotropy getMaxAnisotropy() const {
            return samplerSettings.maxAnisotropy;
        }

        [[nodiscard]] SamplerSettings::MagnificationFilterType getMagnificationFilterType() const {
            return samplerSettings.magnificationFilterType;
        }

        [[nodiscard]] SamplerSettings::MinificationFilterType getMinificationFilterType() const {
            return samplerSettings.minificationFilterType;
        }

        [[nodiscard]] SamplerSettings::SamplerMipmapMode getSamplerMipmapMode() const {
            return samplerSettings.samplerMipmapMode;
        }

        [[nodiscard]] SamplerSettings::OutOfBoundsTexelCoordinateAddressMode getAddressMode() const {
            return samplerSettings.addressMode;
        }

        [[nodiscard]] SamplerSettings::PercentageCloserFilteringCompareOperation getPcfCompareOperation() const {
            return samplerSettings.pcfCompareOperation;
        }

        [[nodiscard]] const std::shared_ptr<Image> &getSampledImage() const {
            return sampledImage;
        }

        [[nodiscard]] const SamplerSettings &getSamplerSettings() const {
            return samplerSettings;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            // TODO - add transfer dest/source to usages if requested

            return std::make_shared<gir::ImageIR>(gir::ImageIR::CreationInput{
                getName(),
                getUid(),
                gir::GIRSubtype::IMAGE,
                gir::ImageIR::ImageUsage::SAMPLED_TEXTURE,
                sampledImage->getImageData()->getRawDataByteArray(),
                sampledImage->getImageData()->getRawDataSizeInBytes()
            });
        }

        UpdateResult update() override {
            // TODO
            return UpdateResult::SUCCESS;
        }

    private:
        SamplerSettings samplerSettings;

        std::shared_ptr<Image> sampledImage;
    };
}
