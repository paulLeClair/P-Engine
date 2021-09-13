#pragma once

#include "../../utils/UniqueID.hpp"

#include "../../../glm/glm.hpp"

namespace scene {

// maybe all renderables can just follow these vertex format conventions
enum class VertexAttributes : unsigned {
    Position = 0,
    Normal = 1,
    Color = 2,
    UV = 3,
    Tangent = 4,
    Bitangent = 5,
    BoneIndex = 6,
    BoneWeights = 7,
    None
};

enum VertexAttributeFlagBits {
    PositionBit = 1 << unsigned(VertexAttributes::Position),
    NormalBit = 1 << unsigned(VertexAttributes::Normal),
    ColorBit = 1 <<unsigned(VertexAttributes::Color),
    UVBit = 1 << unsigned(VertexAttributes::UV),
    TangentBit = 1 << unsigned(VertexAttributes::Tangent),
    BitangentBit = 1 << unsigned(VertexAttributes::Bitangent),
    BoneIndexBit = 1 << unsigned(VertexAttributes::BoneIndex),
    BoneWeightsBit = 1 << unsigned(VertexAttributes::BoneWeights)
};
using VertexAttributeFlags = unsigned int;

namespace VertexAttributeTypes {
  using PositionType = glm::vec3;
  using NormalType = glm::vec3;
  using ColorType = glm::vec4; // maybe i should split this into diffuse and specular?
  using UVType = glm::vec2;
  using TangentType = glm::vec3;
  using BitangentType = glm::vec3;
  // other attribute types: todo for now
}

class Renderable {
  public:
    ~Renderable() = default;

    // some simple functions for determining the type of renderable
    virtual bool isMesh() {
        return false;
    }

    virtual bool isPatchList() {
      return false;
    }

    virtual bool isSubMesh() {
      return false;
    }

    const UniqueID::uid_t &getID() {
      return _id.get();
    }

  protected:
    Renderable() {
        // empty ctor should be fine, these shouldn't maintain much state
    }

    // gonna try giving each renderable a fundamental uniqueID, which should be used in lots of places
    UniqueID _id;

};
    
}
