//
// Created by paull on 2023-01-08.
//

#pragma once

#include <memory>
#include "../SceneResource.hpp"
#include "../Image/Image.hpp"
#include "../Sampler/Sampler.hpp"

/**
 * This corresponds to whatever backend resources are required to create particular texture instances that
 * can be sampled by shaders.
 *
 * This at least supports all the stuff you need to make a sampler in Vulkan!
 */
class Texture : public SceneResource {
public:
    enum class ResourceConfiguration {
        SAMPLER_AND_SAMPLED_IMAGE,
        SAMPLER_ONLY
    };

    struct CreationInput {
        std::shared_ptr<Scene> parentScene;
        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        ResourceConfiguration resourceConfiguration = ResourceConfiguration::SAMPLER_ONLY;

        std::shared_ptr<Sampler> sampler;

        std::shared_ptr<Image> sampledImage = nullptr;
    };

    explicit Texture(const CreationInput &creationInput) : parentScene(creationInput.parentScene),
                                                           name(creationInput.name),
                                                           uniqueIdentifier(creationInput.uniqueIdentifier),

                                                           resourceConfiguration(creationInput.resourceConfiguration),
                                                           sampledImage(creationInput.sampledImage),
                                                           sampler(creationInput.sampler) {
    }

    ~Texture() override = default;

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] Sampler::MipLevelOfDetailBias getMipLevelOfDetailBias() const {
        return sampler->getMipLevelOfDetailBias();
    }

    [[nodiscard]] Sampler::MaxAnisotropy getMaxAnisotropy() const {
        return sampler->getMaxAnisotropy();
    }

    [[nodiscard]] Sampler::MagnificationFilterType getMagnificationFilterType() const {
        return sampler->getMagnificationFilterType();
    }

    [[nodiscard]] Sampler::MinificationFilterType getMinificationFilterType() const {
        return sampler->getMinificationFilterType();
    }

    [[nodiscard]] Sampler::SamplerMipmapMode getSamplerMipmapMode() const {
        return sampler->getSamplerMipmapMode();
    }

    [[nodiscard]] Sampler::OutOfBoundsTexelCoordinateAddressMode getAddressMode() const {
        return sampler->getAddressMode();
    }

    [[nodiscard]] Sampler::PercentageCloserFilteringCompareOperation getPcfCompareOperation() const {
        return sampler->getPcfCompareOperation();
    }

    [[nodiscard]] ResourceConfiguration getResourceConfiguration() const {
        return resourceConfiguration;
    }

    [[nodiscard]] const std::shared_ptr<Image> &getSampledImage() const {
        return sampledImage;
    }

    [[nodiscard]] const std::shared_ptr<Sampler> &getSampler() const {
        return sampler;
    }

    UpdateResult update() override {
        // TODO
        return UpdateResult::SUCCESS;
    }

private:
    std::shared_ptr<Scene> parentScene;

    std::string name;

    PUtilities::UniqueIdentifier uniqueIdentifier;

    ResourceConfiguration resourceConfiguration;

    std::shared_ptr<Sampler> sampler;

    std::shared_ptr<Image> sampledImage = nullptr;
};
