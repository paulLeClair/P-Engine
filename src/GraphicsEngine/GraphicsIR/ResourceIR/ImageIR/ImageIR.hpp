#pragma once

#include <memory>
#include <unordered_set>
#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../../../utilities/RawDataContainer/RawDataContainer.hpp"

namespace pEngine::girEngine::gir {
    /**
     * Similar to buffer object, this should be a platform-agnostic
     * class for image data (including textures now)
    */
    class ImageIR final : public GraphicsIntermediateRepresentation {
    public:
        enum class ImageUsage {
            UNDEFINED,
            STORAGE_IMAGE,
            SAMPLED_TEXTURE,
            COLOR_ATTACHMENT,
            INPUT_ATTACHMENT,
            DEPTH_ATTACHMENT,
            STENCIL_ATTACHMENT,
            DEPTH_STENCIL_ATTACHMENT
        };

        enum class ImageFormat {
            UNDEFINED,
            // TODO - add a bunch of image formats! might want to break this up or move it if it's too much
        };

        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            const ImageUsage imageUsage = ImageUsage::UNDEFINED;

            const unsigned char *dataPointer = nullptr;
            const size_t numberOfBytesToCopy = 0;

            const bool disableTransferSource = false;
            const bool disableTransferDestination = false;

            const size_t width = 0u;
            const size_t height = 0u;
        };

        explicit ImageIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              imageUsage(creationInput.imageUsage),
              imageData(std::make_unique<util::RawDataContainer>(
                  util::RawDataContainer::CreationInput{
                      creationInput.name,
                      creationInput.uid,
                      const_cast<unsigned char *>(creationInput.dataPointer),
                      creationInput.numberOfBytesToCopy
                  })),
              disableTransferSource(creationInput.disableTransferSource),
              disableTransferDestination(creationInput.disableTransferDestination),
              width(creationInput.width),
              height(creationInput.height) {
        }

        ~ImageIR() override = default;

        [[nodiscard]] ImageUsage getImageUsage() const {
            return imageUsage;
        }

        [[nodiscard]] const util::RawDataContainer &getImageData() const {
            return *imageData;
        }

        [[nodiscard]] ImageFormat getFormat() const {
            return imageFormat;
        }

        [[nodiscard]] bool isTransferSourceDisabled() const {
            return disableTransferSource;
        }

        [[nodiscard]] bool isTransferDestinationDisabled() const {
            return disableTransferDestination;
        }

        [[nodiscard]] uint32_t getWidth() const {
            return width;
        }

        [[nodiscard]] uint32_t getHeight() const {
            return height;
        }

    private:
        ImageUsage imageUsage;
        std::unique_ptr<util::RawDataContainer> imageData;

        // image format - TODO
        ImageFormat imageFormat = ImageFormat::UNDEFINED;

        const bool disableTransferSource = false;
        const bool disableTransferDestination = false;

        // TODO - re-evaluate the way we represent image dimensions
        const uint32_t width;
        const uint32_t height;

        // TODO - add the rest of the image information
    };
}
