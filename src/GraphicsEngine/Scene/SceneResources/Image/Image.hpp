//
// Created by paull on 2022-09-21.
//

#pragma once

#include "../SceneResource.hpp"
#include "../../../../EngineCore/utilities/RawDataContainer/RawDataContainer.hpp"
#include "../TexelFormat/TexelFormat.hpp"

#include <unordered_set>
#include <memory>

using namespace PUtilities;
// the fact that we'd have to make each image class have a
// texel type template argument might make this way of handling
// image formats a no-go;

class Image : public SceneResource {
public:
    // TODO - add configuration for 2D, 3D, 4D, ... images

    struct ImageExtent2D {
        unsigned int x;
        unsigned int y;

        ImageExtent2D(unsigned int x, unsigned int y) : x(x), y(y) {}
    };

    enum class ImageUsage : unsigned int {
        TransferSource = 1,
        TransferDestination = 2,
        ShaderSampled = 3,
        Storage = 4,
        ColorAttachment = 5,
        DepthStencilAttachment = 6,
        TransientAttachment = 7,
        InputAttachment = 8
    };

    struct CreationInput {
        std::shared_ptr<Scene> scene;
        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;
        std::shared_ptr<TexelFormat> format;

        ImageExtent2D imageExtent;

        // TODO - add mipmap functionality

        // TODO - array/image layers

        // TODO - image tiling

        unsigned int numberOfSamples;

        std::unordered_set<ImageUsage> imageUsages;

        unsigned char *initialTexelData = nullptr;
        unsigned long numberOfInitializationDataTexels = 0;
    };

    explicit Image(const CreationInput &createInfo) : name(createInfo.name),
                                                      uniqueIdentifier(createInfo.uniqueIdentifier),
                                                      format(createInfo.format),
                                                      imageDimensions(createInfo.imageExtent),
                                                      numberOfSamples(createInfo.numberOfSamples),
                                                      imageUsages(createInfo.imageUsages) {
        rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
                name,
                uniqueIdentifier,
                createInfo.initialTexelData,
                createInfo.numberOfInitializationDataTexels * format->getTexelSizeInBytes()
        });
    }

    ~Image() override = default;

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


        return UpdateResult::SUCCESS;
    }

    std::shared_ptr<TexelFormat> getFormat() {
        return format;
    }

    const std::unordered_set<ImageUsage> &getImageUsages() {
        return imageUsages;
    }

    ImageExtent2D getImageExtent2D() {
        return imageDimensions;
    }

    [[nodiscard]] unsigned int getNumberOfSamples() const {
        return numberOfSamples;
    }

protected:
    std::shared_ptr<Scene> parentScene;

    std::string name;

    PUtilities::UniqueIdentifier uniqueIdentifier;

    std::shared_ptr<TexelFormat> format;

    ImageExtent2D imageDimensions;

    unsigned int numberOfSamples;

    std::unordered_set<ImageUsage> imageUsages = {};

    std::shared_ptr<RawDataContainer> rawDataContainer;
};
