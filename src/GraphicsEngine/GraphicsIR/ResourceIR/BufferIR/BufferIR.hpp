#pragma once

#include <memory>
#include <unordered_set>
#include <utility>

#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../ResourceIR/FormatIR/FormatIR.hpp"
#include "../../../../utilities/ByteArray/ByteArray.hpp"

#include <boost/optional.hpp>

namespace pEngine::girEngine::gir {
    /**
     * This should correspond to a buffer object; most graphics APIs
     * represent these relatively similarly so hopefully we can get a nice
     * platform-agnostic design goin
    */
    struct BufferIR : public GraphicsIntermediateRepresentation {
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

        BufferIR() : GraphicsIntermediateRepresentation({}), usage(BufferUsage::UNKNOWN) {
        }

        BufferIR(const std::string &name,
                 util::UniqueIdentifier uid,
                 GIRSubtype subtype,
                 BufferUsage usage,
                 uint32_t bindingIndex, // this index should be either for vertex input or descriptor set binding index
                 const uint8_t *initialRawDataPtr = nullptr,
                 const uint32_t initialRawDataSizeInBytes = 0u,
                 const boost::optional<uint32_t> &maximumBufferSizeInBytes = boost::none,
                 const boost::optional<gir::resource::FormatIR> &elementFormat = boost::none,
                 const boost::optional<uint32_t> &elementCount = boost::none)
            : GraphicsIntermediateRepresentation({name, uid, subtype}),
              usage(usage),
              bindingIndex(bindingIndex),
              rawDataContainer(util::ByteArray::CreationInput{
                  name + "Raw Data Container",
                  uid,
                  initialRawDataPtr,
                  initialRawDataSizeInBytes
              }),
              maximumBufferSizeInBytes(maximumBufferSizeInBytes),
              elementFormat(elementFormat),
              elementCount(elementCount) {
        }

        ~BufferIR() override = default;

        BufferUsage usage; // TODO - replace this with a bitmap probably

        /**
         * Depending on whether the buffer is for geometry or has to use descriptor sets for binding,
         * this should correspond to the vertex input binding index in the first case, or the
         * descriptor set binding index in the second case.
         */
        uint32_t bindingIndex = 0;

        util::ByteArray rawDataContainer;

        /**
         * When you are creating uninitialized (empty) buffer, you have to provide
         * an upper bound on how big your data is (for the time being). \n\n
         * I could probably rename this to being something like "initialCapacity" or something
         * because in effect that's what it's being used for
         */
        boost::optional<uint32_t> maximumBufferSizeInBytes = boost::none;

        /**
         * (for future functionality) With formatted data, you can specify the data format
         * that the data is in. Note that this is only really useful for things like texel buffers
         * which are not supported as of the single-animated-model demo
         */
        boost::optional<gir::resource::FormatIR> elementFormat = boost::none;

        /**
         * For data that stores an array of struct elements, we can have a space to store this
         */
        boost::optional<uint32_t> elementCount = boost::none;
    };
}
