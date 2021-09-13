#pragma once

#include "../../../utils/UniqueID.hpp"

#include <string>

namespace scene {

struct MaterialCreateInfo {
    // include some material create info!


};

class Material {
  public:
    Material(const MaterialCreateInfo &info);
    ~Material() = default;

    // for the most part, setting up materials should be done as part of filling in the meshcreateinfo struct
    
    // materials are meant to apply to specific geometry, and should be hooked in together with the meshes
        // so maybe we can include a named set of materials for each mesh?

    const UniqueID::uid_t &getID() {
        return id_.get();
    }

  private:
    std::string name_;
    UniqueID id_;

};

}