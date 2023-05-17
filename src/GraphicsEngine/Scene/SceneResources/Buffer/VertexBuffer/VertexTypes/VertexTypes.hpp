//
// Created by paull on 2023-03-31.
//

#pragma once

namespace VertexAttributeTypes {
    using PositionType = glm::vec3;
    using NormalType = glm::vec3;
    using ColorType = glm::vec4;// maybe split this into diffuse and specular?
    using TextureCoordinatesType = glm::vec2;
    using TangentType = glm::vec3;
    using BitangentType = glm::vec3;
    // other attribute types: todo for now (i imagine raytracing etc will require more)
}
using namespace VertexAttributeTypes;

enum class VertexTypeToken {
    UNKNOWN,
    POSITION_ONLY,
    POSITION_COLOR,
    POSITION_COLOR_NORMAL,
    POSITION_COLOR_NORMAL_UV,
    POSITION_COLOR_NORMAL_UV_TANGENT,
    POSITION_COLOR_NORMAL_UV_TANGENT_BITANGENT
};

namespace VertexTypes {

    struct PositionOnlyVertex {
        PositionType position;

        bool operator==(const PositionOnlyVertex &rhs) const {
            return position == rhs.position;
        }

        bool operator!=(const PositionOnlyVertex &rhs) const {
            return !(rhs == *this);
        }
    };

    struct PositionColorVertex {
        PositionType position;
        ColorType color;

        bool operator==(const PositionColorVertex &rhs) const {
            return position == rhs.position &&
                   color == rhs.color;
        }

        bool operator!=(const PositionColorVertex &rhs) const {
            return !(rhs == *this);
        }
    };

    struct PositionColorNormalVertex {
        PositionType position;
        ColorType color;
        NormalType normal;

        bool operator==(const PositionColorNormalVertex &rhs) const {
            return position == rhs.position &&
                   color == rhs.color &&
                   normal == rhs.normal;
        }

        bool operator!=(const PositionColorNormalVertex &rhs) const {
            return !(rhs == *this);
        }
    };

    struct PositionColorNormalUVVertex {
        PositionType position;
        ColorType color;
        NormalType normal;
        TextureCoordinatesType textureCoordinates;

        bool operator==(const PositionColorNormalUVVertex &rhs) const {
            return position == rhs.position &&
                   color == rhs.color &&
                   normal == rhs.normal &&
                   textureCoordinates == rhs.textureCoordinates;
        }

        bool operator!=(const PositionColorNormalUVVertex &rhs) const {
            return !(rhs == *this);
        }
    };

    struct PositionColorNormalUVTangentVertex {
        PositionType position;
        ColorType color;
        NormalType normal;
        TextureCoordinatesType textureCoordinates;
        TangentType tangent;

        bool operator==(const PositionColorNormalUVTangentVertex &rhs) const {
            return position == rhs.position &&
                   color == rhs.color &&
                   normal == rhs.normal &&
                   textureCoordinates == rhs.textureCoordinates &&
                   tangent == rhs.tangent;
        }

        bool operator!=(const PositionColorNormalUVTangentVertex &rhs) const {
            return !(rhs == *this);
        }
    };

    struct PositionColorNormalUVTangentBitangentVertex {
        PositionType position;
        ColorType color;
        NormalType normal;
        TextureCoordinatesType textureCoordinates;
        TangentType tangent;
        BitangentType bitangent;

        bool operator==(const PositionColorNormalUVTangentBitangentVertex &rhs) const {
            return position == rhs.position &&
                   color == rhs.color &&
                   normal == rhs.normal &&
                   textureCoordinates == rhs.textureCoordinates &&
                   tangent == rhs.tangent &&
                   bitangent == rhs.bitangent;
        }

        bool operator!=(const PositionColorNormalUVTangentBitangentVertex &rhs) const {
            return !(rhs == *this);
        }
    };
};

// TODO - expand with other vertex types that we want to support
