#pragma once

#include <memory>
#include <unordered_set>

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../../../utilities/RawDataContainer/RawDataContainer.hpp"

namespace pEngine::girEngine::gir {
    /**
     * This should correspond to a buffer object; most graphics APIs
     * represent these relatively similarly so hopefully we can get a nice
     * platform-agnostic design goin
    */
    class BufferIR : public GraphicsIntermediateRepresentation {
    public:
        /**
         * I think using this simple token for what the buffer will be used
         * for is a good design for now; should be easy enough
         * for the backend to build the appropriate structures based on these
         * common usages. Obviously it's on the user to guarantee that they're using
         * the right tool for the given job, but it shouldn't hold their hand beyond
         * providing some guidance about what the different types are good for (TODO)\n\n
         *
         * For now the terminology I'm using is Vulkan-centric but tbh
         * Vulkan is the clearest and most complete API and is the best for
         * cross-platform vs metal and dx12 so I think it can win out here. \n\n
         *
         * If I come up with better names for these different usages though
         * I'll definitely try them out.
         */
        enum class BufferUsage {
            UNKNOWN,
            UNIFORM_BUFFER,
            DYNAMIC_UNIFORM_BUFFER,
            STORAGE_BUFFER,
            DYNAMIC_STORAGE_BUFFER,
            TEXEL_BUFFER,
            STORAGE_TEXEL_BUFFER,
            VERTEX_BUFFER,
            INDEX_BUFFER
            // TODO - any other common uses
        };

        /**
         * 03-19 ISSUE FOUND: we need to make special accomodations for the transfer usages, they aren't like the others
         * in that they are usually paired with another usage. I think the easiest way is to make toggleable flags and
         * have every buffer be transfer destination by default and then have transfer source be disabled by default
         */
        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            const BufferUsage usage;

            const unsigned char *dataPointer = nullptr;
            const size_t numberOfBytesToCopy = 0;
        };

        explicit BufferIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              usage(creationInput.usage),
              rawDataContainer(
                  std::make_unique<util::RawDataContainer>(
                      util::RawDataContainer::CreationInput{
                          creationInput.name,
                          creationInput.uid,
                          const_cast<unsigned char *>(creationInput.dataPointer),
                          creationInput.numberOfBytesToCopy
                      })) {
        }

        BufferIR(const BufferIR &other);

        ~BufferIR() override = default;

        [[nodiscard]] const BufferUsage &getUsage() const {
            return usage;
        }

        [[nodiscard]] const util::RawDataContainer &getRawDataContainer() const {
            return *rawDataContainer;
        }

    private:
        const BufferUsage usage; // TODO - replace this with a bitmap probably

        std::unique_ptr<util::RawDataContainer> rawDataContainer;
    };
}
