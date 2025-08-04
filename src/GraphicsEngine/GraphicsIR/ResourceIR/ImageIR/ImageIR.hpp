#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../../../utilities/ByteArray/ByteArray.hpp"
#include "../FormatIR/FormatIR.hpp"
#include <vulkan/vulkan.h>
#include <boost/optional.hpp>

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
            DEPTH_STENCIL_ATTACHMENT,
            SWAPCHAIN_IMAGE
        };

        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            const ImageUsage imageUsage = ImageUsage::UNDEFINED;

            const resource::FormatIR format = resource::FormatIR::UNDEFINED;

            const uint8_t *dataPointer = nullptr;
            const uint32_t numberOfBytesToCopy = 0;

            const uint32_t width = 0u;
            const uint32_t height = 0u;

            const bool disableTransferSource = false;
            const bool disableTransferDestination = false;

            boost::optional<uint32_t> swapchainImageIndex = boost::none;

        };

        explicit ImageIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  imageUsage(creationInput.imageUsage),
                  imageData(util::ByteArray(
                          util::ByteArray::CreationInput{
                                  creationInput.name,
                                  creationInput.uid,
                                  const_cast<unsigned char *>(creationInput.dataPointer),
                                  creationInput.numberOfBytesToCopy
                          })),
                  disableTransferSource(creationInput.disableTransferSource),
                  disableTransferDestination(creationInput.disableTransferDestination),
                  width(creationInput.width),
                  height(creationInput.height),
                  imageFormat(creationInput.format),
                  swapchainImageIndex(creationInput.swapchainImageIndex) {
        }

        ImageIR() : GraphicsIntermediateRepresentation() {

        }

        ~ImageIR() override = default;

        ImageUsage imageUsage = ImageUsage::UNDEFINED;
        util::ByteArray imageData = {};

        // image format
        resource::FormatIR imageFormat = resource::FormatIR::UNDEFINED;

        const bool disableTransferSource = false;
        const bool disableTransferDestination = false;

        // TODO - re-evaluate the way we represent image dimensions
        const uint32_t width = 0;
        const uint32_t height = 0;

        // TODO - add the rest of the image information
//        boost::optional<VkImage> swapchainImageHandle = boost::none;
        boost::optional<uint32_t> swapchainImageIndex = boost::none;
    };
}
