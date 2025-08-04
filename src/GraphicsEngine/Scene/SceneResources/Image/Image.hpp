//
// Created by paull on 2022-09-21.
//

#pragma once

#include "../SceneResource.hpp"
#include "../../../../utilities/ByteArray/ByteArray.hpp"
#include "../../../GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp"
#include "../formats/ResourceFormat/ResourceFormat.hpp"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <boost/optional.hpp>

using namespace pEngine::util;
// the fact that we'd have to make each image class have a
// texel type template argument might make this way of handling
// image formats a no-go;

namespace pEngine::girEngine::scene {
    class Image {
    public:
        // TODO - add configuration for 2D, 3D, 4D, ... images

        struct ImageExtent2D {
            unsigned int x;
            unsigned int y;
        };

        enum class ImageUsage : unsigned int {
            UNDEFINED,
            SampledTextureImage,
            Storage,
            ColorAttachment,
            DepthStencilAttachment,
            TransientAttachment, // these are TODO for a while
            InputAttachment,
            SwapchainRenderTarget
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
            unsigned int numberOfMipMapLevels = 0;
            bool enableBlitting = true;
        };

        /**
         * This will be used to configure any multi-layer images that the user wants.
         *
         * Default constructor / no specification should result in single layers only
         */
        struct ImageArrayConfiguration {
            unsigned int numberOfArrayLevels = 1;
            // TODO
        };

        /**
         * TODO -> make a lot of these optional, provide reasonable defaults etc
         */
        struct CreationInput {
            std::string name;

            util::UniqueIdentifier uid;

            ResourceFormat format = ResourceFormat::UNDEFINED;

            ImageUsage imageUsages = ImageUsage::UNDEFINED;

            uint32_t width = 0u;
            uint32_t height = 0u;

            uint8_t *initialImageData = nullptr;
            uint32_t initialImageDataSizeInBytes = 0;

            boost::optional<ImageExtent2D> imageExtent = {};

            boost::optional<MipMapConfiguration> mipMapConfiguration = boost::none;

            boost::optional<ImageArrayConfiguration> arrayLevelConfiguration = boost::none;

            boost::optional<TexelTilingArrangement> texelTiling = boost::none;

            boost::optional<unsigned int> numberOfSamples = boost::none;


            boost::optional<uint32_t> swapchainIndex = boost::none;
        };

        explicit Image(const CreationInput &createInfo) : name(createInfo.name),
                                                          uniqueIdentifier(createInfo.uid),
                                                          format(createInfo.format),
                                                          imageDimensions(createInfo.imageExtent.get_value_or({})),
                                                          numberOfSamples(createInfo.numberOfSamples.get_value_or({})),
                                                          imageUsage(createInfo.imageUsages),
                                                          texelTiling(createInfo.texelTiling.get_value_or({})),
                                                          mipMapConfiguration(
                                                              createInfo.mipMapConfiguration.get_value_or({})),
                                                          imageArrayConfiguration(
                                                              createInfo.arrayLevelConfiguration.get_value_or({})),
                                                          width(createInfo.width),
                                                          height(createInfo.height),
                                                          swapchainIndex(createInfo.swapchainIndex) {
            imageData = ByteArray(ByteArray::CreationInput{
                createInfo.name,
                createInfo.uid,
                createInfo.initialImageData,
                createInfo.initialImageDataSizeInBytes
            });
        }

        Image() : Image(CreationInput{}) {
        }

        Image(const Image &other)
            : name(other.name),
              uniqueIdentifier(other.uniqueIdentifier),
              format(other.format),
              imageDimensions(other.imageDimensions),
              numberOfSamples(other.numberOfSamples),
              texelTiling(other.texelTiling),
              mipMapConfiguration(other.mipMapConfiguration),
              imageArrayConfiguration(other.imageArrayConfiguration),
              imageUsage(other.imageUsage),
              width(other.width),
              height(other.height),
              imageData(other.imageData) {
        }

        ~Image() = default;

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

        [[nodiscard]] ResourceFormat getFormat() const {
            return format;
        }

        [[nodiscard]] const ImageExtent2D &getImageDimensions() const {
            return imageDimensions;
        }

        [[nodiscard]] const ImageArrayConfiguration &getImageArrayConfiguration() const {
            return imageArrayConfiguration;
        }

        [[nodiscard]] const ByteArray &getImageData() const {
            return imageData;
        }

        static gir::ImageIR::ImageUsage getGIRImageUsageFromSceneImageUsage(const ImageUsage &usage) {
            static const std::unordered_map<ImageUsage, gir::ImageIR::ImageUsage> IMAGE_USAGE_CONVERSION_MAP = {
                {ImageUsage::SampledTextureImage, gir::ImageIR::ImageUsage::SAMPLED_TEXTURE},
                {ImageUsage::Storage, gir::ImageIR::ImageUsage::STORAGE_IMAGE},
                {ImageUsage::ColorAttachment, gir::ImageIR::ImageUsage::COLOR_ATTACHMENT},
                {ImageUsage::DepthStencilAttachment, gir::ImageIR::ImageUsage::DEPTH_STENCIL_ATTACHMENT},
                {ImageUsage::InputAttachment, gir::ImageIR::ImageUsage::INPUT_ATTACHMENT},
                {ImageUsage::TransientAttachment, gir::ImageIR::ImageUsage::UNDEFINED},
                {ImageUsage::SwapchainRenderTarget, gir::ImageIR::ImageUsage::SWAPCHAIN_IMAGE}
            };
            return IMAGE_USAGE_CONVERSION_MAP.at(usage);
        }

        gir::ImageIR bakeImage() const {
            return gir::ImageIR(
                gir::ImageIR::CreationInput{
                    name,
                    uniqueIdentifier,
                    gir::GIRSubtype::IMAGE,
                    getGIRImageUsageFromSceneImageUsage(imageUsage),
                    static_cast<const gir::resource::FormatIR>(format),
                    // TODO - probably just use GIR usages in this class so we don't have to pass by value
                    imageData.getRawDataByteArray(),
                    static_cast<uint32_t>(imageData.getRawDataSizeInBytes()),
                    width,
                    height,
                    false,
                    false,
                    swapchainIndex
                });
        }

        std::string name;

        UniqueIdentifier uniqueIdentifier;

    private:
        ResourceFormat format;

        ImageExtent2D imageDimensions;

        uint32_t numberOfSamples = 0u;

        TexelTilingArrangement texelTiling;

        MipMapConfiguration mipMapConfiguration;

        ImageArrayConfiguration imageArrayConfiguration;

        ImageUsage imageUsage;

        uint32_t width = 0u;
        uint32_t height = 0u;

        ByteArray imageData;

        boost::optional<uint32_t> swapchainIndex = boost::none;
    };
}
