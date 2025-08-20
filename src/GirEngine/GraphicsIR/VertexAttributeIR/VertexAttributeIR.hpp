//
// Created by paull on 2024-05-03.
//

#pragma once

#include <string>
#include <boost/optional/optional.hpp>
#include <utility>

// ugh coming back to this I am kind of appalled at it.
// the idea of going through and making sure I set these up properly
// for each format sounds like booty.
// on the other hand a simple hardcoded static map could be somewhat doable

// it appears that this VertexAttributeIR class isn't even really used;
// it's a good example of why I shouldn't write code without having a bit of
// background on the domain lol

namespace pEngine::girEngine::gir::vertex {

    // TODO -> consider making this a GIR subclass (or whatever the alternative is if we remove inheritance)
    struct VertexAttributeIR {
        VertexAttributeIR(const VertexAttributeIR &other) = default;

        VertexAttributeIR() = default;

        // ideally this should match up with the attribute name in the actual shader; not sure if it'll be actually used
        std::string attributeName;

        // for simplicity, I'll just use a 1-1 matching enum with the scene attribute usage label;
        enum class AttributeUsage {
            UNDEFINED,
            POSITION,
            COLOR,
            NORMAL,
            TANGENT,
            BITANGENT,
            UV,
            ANIMATION_BONE_INDICES,
            ANIMATION_BONE_WEIGHTS
            // TODO -> other commonly supported attribute types (reflectance, etc)
        };

        /**
         * This is the index of the vertex input binding configuration that this attribute is mapped to
         */
        unsigned binding = 0;

        /**
         * This is the index of the shader resource slot
         */
        unsigned attributeShaderLocation = 0;

        gir::resource::FormatIR attributeFormat = gir::resource::FormatIR::UNDEFINED;

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

        boost::optional<AttributeUsage> usageLabel = boost::none;

        VertexAttributeIR(const std::string &attributeName,
                          unsigned int binding,
                          unsigned int attributeShaderLocation,
                          resource::FormatIR attributeFormat,
                          unsigned int attributeByteOffset,
                          const boost::optional<AttributeUsage> &usageLabel) : attributeName(attributeName),
                                                                               binding(binding),
                                                                               attributeShaderLocation(
                                                                                       attributeShaderLocation),
                                                                               attributeFormat(attributeFormat),
                                                                               attributeByteOffset(attributeByteOffset),
                                                                               usageLabel(usageLabel) {}
    };

};