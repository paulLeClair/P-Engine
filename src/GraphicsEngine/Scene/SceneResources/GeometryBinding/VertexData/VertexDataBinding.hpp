//
// Created by paull on 2024-10-01.
//

#pragma once

#include <boost/optional.hpp>
#include "../../../../../utilities/ByteArray/ByteArray.hpp"
#include "../../formats/ResourceFormat/ResourceFormat.hpp"

namespace pEngine::girEngine::scene::geometry {
    /**
     * At least for the short term, this will encode the supported attribute types;
     * at least for Assimp-imported models, it controls the particular assimp data that
     * is copied into this particular attribute's position in its particular vertex input binding.
     */
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
            * This contains information about one particular attribute for one particular vertex buffer binding.
            *
            * Note: the vertex data binding index is implicitly given by the position of a particular AttributeDescription
            * inside of the vector of AttributeDescriptions inside the VertexDataBinding class.
            */
    struct AttributeDescription {
        // as far as the actual implementation, I'll just continue the "pre-alpha" pattern of
        // closely matching the GIR class;

        /**
         * I'm not sure what this will be used for; maybe we can have it set up so that you can
         * use it to directly specify a variable name in a shader, and we can use that for automation. \n\n
         * (seems like a pretty convenient feature to be able to ignore all the binding/layout info after
         * initial shader specification and just build the binding by giving input variable names that match the shader;
         * on the other hand you don't want to pigeonhole 1 binding-per-vert-shader because that's ugly too;
         * the ability to do both is probably best)
         */
        std::string attributeName;

        /**
         * This is an index into the shader resource interface slot array.
         *
         * See https://docs.vulkan.org/spec/latest/chapters/interfaces.html#interfaces-iointerfaces-locations
         */
        unsigned shaderResourceLocation = 0;

        /**
         * This is an enum which describes the format of packed data.
         *
         * This currently maps 1-1 with Vulkan's format enum because the engine
         * is currently focused on Vulkan; if it turns out there's a less-annoying way
         * to do this then I'll switch it out eventually
         */
        ResourceFormat attributeFormat = ResourceFormat::UNDEFINED;

        /**
         * This is the byte offset into the (presumably interleaved) vertex itself;
         * for single-attribute bindings this should always be 0, but if you interleave
         *
         */
        unsigned vertexOffset = 0;

        /**
         * Currently this is used for assimp model imports
         */
        boost::optional<AttributeUsage> usageLabel = boost::none;

        /**
         * This is the size of a particular attribute, since our Format itself does not currently have an easy way to
         * provide its own size.
         */
        uint32_t size;
    };

    /**
     * The @GeometryBinding struct which packages these up is meant to correspond to one particular
     * binding point for a vertex buffer (meaning it just stores the attribute descriptions for this binding)
     */
    struct VertexDataBinding {
        std::vector<AttributeDescription> attributes = {};
        unsigned vertexStride = 0;
    };
}
