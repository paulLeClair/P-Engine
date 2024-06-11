//
// Created by paull on 2022-09-21.
//

#pragma once

#include "../SceneResource.hpp"
#include "../../../../utilities/RawDataContainer/RawDataContainer.hpp"
#include "../formats/TexelFormat/TexelFormat.hpp"
#include "../../../GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp"

#include <unordered_map>
#include <unordered_set>
#include <memory>

using namespace pEngine::util;
// the fact that we'd have to make each image class have a
// texel type template argument might make this way of handling
// image formats a no-go;

namespace pEngine::girEngine::scene {
    class Image : public scene::Resource {
    public:
        // TODO - add configuration for 2D, 3D, 4D, ... images

        struct ImageExtent2D {
            unsigned int x;
            unsigned int y;
        };

        enum class ImageUsage : unsigned int {
            SampledTextureImage,
            Storage,
            ColorAttachment,
            DepthStencilAttachment,
            TransientAttachment, // these are TODO for a while
            InputAttachment
        };

        /**
         * This describes how texels are laid out in memory;
         * either row major or column major
         *
         * TODO - evaluate what options we actually need to provide here
         */
        enum class TexelTilingArrangement {
            BACKEND_OPTIMAL,
            LINEAR_ROW_MAJOR,
            LINEAR_COLUMN_MAJOR
        };

        /**
         * This is still mostly TODO, since we aren't going to worry about mipmaps for this current milestone.
         *
         * Default behavior should be that default will disable mip mapping, so you only
         * specify it if you want mipmapping to happen.
         *
         */
        struct MipMapConfiguration {
            bool mipmapEnabled = false;
            unsigned int numberOfMipMapLevels = 0;
            bool enableMipMapBlitting = true;
        };

        /**
         * This will be used to configure any multi-layer images that the user wants.
         *
         * Default constructor / no specification should result in single layers only
         */
        struct ImageArrayConfiguration {
            bool isArrayImage = false;
            unsigned int numberOfArrayLevels = 1;
            // TODO
        };

        struct CreationInput : public Resource::CreationInput {
            TexelFormat format;

            ImageExtent2D imageExtent;

            MipMapConfiguration mipMapConfiguration;

            ImageArrayConfiguration arrayLevelConfiguration;

            TexelTilingArrangement texelTiling;

            unsigned int numberOfSamples;

            ImageUsage imageUsages;

            unsigned char *initialImageData = nullptr;
            unsigned long initialImageDataSizeInBytes = 0;
        };

        explicit Image(const CreationInput &createInfo) : Resource(createInfo),
                                                          format(createInfo.format),
                                                          imageDimensions(createInfo.imageExtent),
                                                          numberOfSamples(createInfo.numberOfSamples),
                                                          imageUsage(createInfo.imageUsages),
                                                          texelTiling(createInfo.texelTiling),
                                                          mipMapConfiguration(createInfo.mipMapConfiguration),
                                                          imageArrayConfiguration(createInfo.arrayLevelConfiguration) {
            imageData = std::make_unique<RawDataContainer>(RawDataContainer::CreationInput{
                createInfo.name,
                createInfo.uid,
                createInfo.initialImageData,
                createInfo.initialImageDataSizeInBytes
            });
        }

        ~Image() override = default;

        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        [[nodiscard]] TexelFormat getImageFormat() const {
            return format;
        }

        [[nodiscard]] const ImageUsage &getImageUsages() const {
            return imageUsage;
        }

        [[nodiscard]] ImageExtent2D get2DImageExtent() const {
            return imageDimensions;
        }

        [[nodiscard]] unsigned int getNumberOfSamples() const {
            return numberOfSamples;
        }

        [[nodiscard]] TexelTilingArrangement getTexelTiling() const {
            return texelTiling;
        }

        [[nodiscard]] const MipMapConfiguration &getMipMapConfiguration() const {
            return mipMapConfiguration;
        }

        [[nodiscard]] TexelFormat getFormat() const {
            return format;
        }

        [[nodiscard]] const ImageExtent2D &getImageDimensions() const {
            return imageDimensions;
        }

        [[nodiscard]] const ImageArrayConfiguration &getImageArrayConfiguration() const {
            return imageArrayConfiguration;
        }

        [[nodiscard]] const std::unique_ptr<RawDataContainer> &getImageData() const {
            return imageData;
        }

        static gir::ImageIR::ImageUsage getGIRImageUsageFromSceneImageUsage(const ImageUsage &usage) {
            static const std::unordered_map<ImageUsage, gir::ImageIR::ImageUsage> IMAGE_USAGE_CONVERSION_MAP = {
                {ImageUsage::SampledTextureImage, gir::ImageIR::ImageUsage::SAMPLED_TEXTURE},
                {ImageUsage::Storage, gir::ImageIR::ImageUsage::STORAGE_IMAGE},
                {ImageUsage::ColorAttachment, gir::ImageIR::ImageUsage::COLOR_ATTACHMENT},
                {ImageUsage::DepthStencilAttachment, gir::ImageIR::ImageUsage::DEPTH_STENCIL_ATTACHMENT},
                {ImageUsage::InputAttachment, gir::ImageIR::ImageUsage::INPUT_ATTACHMENT},
                {ImageUsage::TransientAttachment, gir::ImageIR::ImageUsage::UNDEFINED}
            };
            return IMAGE_USAGE_CONVERSION_MAP.at(usage);
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::ImageIR>(
                gir::ImageIR::CreationInput{
                    getName(),
                    getUid(),
                    gir::GIRSubtype::IMAGE,
                    getGIRImageUsageFromSceneImageUsage(imageUsage),
                    // TODO - probably just use GIR usages in this class so we don't have to pass by value
                    imageData->getRawDataByteArray(),
                    imageData->getRawDataSizeInBytes()
                });
        }

    private:
        TexelFormat format;

        ImageExtent2D imageDimensions;

        unsigned int numberOfSamples;

        TexelTilingArrangement texelTiling;

        MipMapConfiguration mipMapConfiguration;

        ImageArrayConfiguration imageArrayConfiguration;

        ImageUsage imageUsage;

        std::unique_ptr<RawDataContainer> imageData;
    };
}
