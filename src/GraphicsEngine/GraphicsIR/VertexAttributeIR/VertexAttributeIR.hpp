//
// Created by paull on 2024-05-03.
//

#pragma once

#include <string>
#include <boost/optional/optional.hpp>
#include <utility>

namespace pEngine::girEngine::gir::vertex {

    enum class AttributeComponentSignedness {
        UNKNOWN,
        SIGNED,
        UNSIGNED
    };

    enum class AttributeComponentSpace {
        UNKNOWN,
        INT,
        FLOAT,
        DOUBLE,
        /**
         * This is a float restrained to either [0,1] or [-1,1] depending on the signedness.
         */
        NORM,
        /**
         * Scaled means you take the (either unsigned or signed) integer value and convert it to a float
         */
        SCALED,
        SRGB,
    };

    enum class AttributeComponentComposition {
        UNKNOWN,
        R,
        RG,
        RGB,
        RGBA,
        AGBR,
        RGBA_2_BIT_ALPHA,
        /**
         * This corresponds to a depth-only format; functionally I'm not sure it's much different than R
         */
        D,
        BGR,
        /**
         * This is for a packed 32-bit, 3-component unsigned float number with a 5-bit exponent; pretty niche, may delete later ;)
         */
        FLOAT_EBGR
    };

    // TODO - actually support different packed formats
    enum class AttributeDataPacking {
        NONE,
        BLOCK,
        PACK8,
        PACK16,
        PACK32,
        ASTC
    };

    struct VertexAttributeIR {
        // ideally this should match up with the attribute name in the actual shader; not sure if it'll be actually used
        std::string attributeName;

        // TODO - verify that this lines up enough with what vulkan needs us to specify
        /**
         * This (I think) is shader-specific, it labels which of the shader's 32-bit interface slots that the
         * attribute memory starts in (in certain cases, you can have a single attribute taking up multiple slots). \n\n
         *
         * This is able to be specified inside the shader, in which case the value from the shader will override this
         * value. We should also be able to support the case that the user has the attributes
         * specified using the "attribute" keyword, which I think forces us to determine a location on a per-binding basis
         *
         * Honestly I think I'll just make this an optional for now, since we might as well allow it to just be reflected from the shader
         * or figured out automatically
         */
        boost::optional<unsigned> attributeShaderLocation = boost::none;

        /**
         * This should correspond to where this attribute is located inside the vertex element memory,
         * which will be used to access this particular attribute inside the vertex shader.\n\n
         *
         * It's very important to set this properly so that the vertex shader accesses the correct spot in memory
         * for this attribute.
         *
         * However I'm not entirely sure that we can't just determine this dynamically... but something like that
         * might require info we don't have. To be safe I'll just leave it as a specified thing for now and re-evaluate later
         */
        unsigned attributeByteOffset = 0u;


        AttributeComponentSignedness componentSignedness = AttributeComponentSignedness::UNKNOWN;


        /**
         * This describes the types of the individual components of this attribute.
         */
        AttributeComponentSpace componentSpace = AttributeComponentSpace::UNKNOWN;

        /**
         * This describes the specific component format of the attribute;
         */
        AttributeComponentComposition componentComposition = AttributeComponentComposition::UNKNOWN;

        /**
         * This should describe the size of the components in bytes; obviously this isn't a true
         * catch-all because there are types with different component sizes b/w them, but they're generally
         * special cases and I can avoid them for now... besides you can probably deduce whether to use those ones in other ways.
         */
        size_t individualComponentSizesInBytes = 0u;

        /**
         * This describes whether the attribute is packed / compressed in some way.
         */
        AttributeDataPacking dataPacking = AttributeDataPacking::NONE;

        VertexAttributeIR(std::string attributeName,
                          const boost::optional<unsigned int> &attributeShaderLocation,
                          unsigned int attributeByteOffset, AttributeComponentSignedness componentSignedness,
                          AttributeComponentSpace componentSpace, AttributeComponentComposition componentComposition,
                          size_t individualComponentSizesInBytes, AttributeDataPacking dataPacking) : attributeName(
                std::move(
                        attributeName)), attributeShaderLocation(attributeShaderLocation), attributeByteOffset(
                attributeByteOffset), componentSignedness(componentSignedness), componentSpace(componentSpace),
                                                                                                      componentComposition(
                                                                                                              componentComposition),
                                                                                                      individualComponentSizesInBytes(
                                                                                                              individualComponentSizesInBytes),
                                                                                                      dataPacking(
                                                                                                              dataPacking) {}
    };

};