//
// Created by paull on 2023-03-06.
//

#pragma once

#include <vector>
#include <string>
#include <variant>
#include <memory>

#include "../../../../lib/glm/glm.hpp"

#include "../../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../../EngineCore/utilities/RawDataContainer/RawDataContainer.hpp"

#include "../SceneResource.hpp"

/**
 * This should map to a push constant in Vulkan, or a cbuffer (etc) in DX12, and so on - basically constant memory
 * that is updated frequently by CPU and is faster for shaders to access (but there's limited space)
 */
class ShaderConstant : public SceneResource {
public:
    enum ShaderStage {
        VERTEX,
        TESSELLATION_CONTROL,
        TESSELLATION_EVALUATION,
        GEOMETRY,
        FRAGMENT,
        MESH,
        // TODO - add raytracing stages (but that will come as a later epic lol)
        ALL_GRAPHICS
    };

    struct CreationInput {
        std::shared_ptr<Scene> scene;

        const std::string &name;
        const PUtilities::UniqueIdentifier uniqueIdentifier;

        std::vector<ShaderStage> shaderStages = {};
        unsigned int offset;
        unsigned int size;

        unsigned char *initializationDataPointer = nullptr;
        unsigned long initializationDataSizeInBytes = 0;


    };

    explicit ShaderConstant(const CreationInput &creationInput) : name(creationInput.name),
                                                                  uniqueIdentifier(
                                                                          creationInput.uniqueIdentifier),
                                                                  shaderStages(
                                                                          creationInput.shaderStages),
                                                                  offset(creationInput.offset),
                                                                  size(creationInput.size) {

    }

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return scene;
    }

    UpdateResult update() override {
        return UpdateResult::FAILURE;
    }

    [[nodiscard]] const std::vector<ShaderStage> &getShaderStages() const {
        return shaderStages;
    }

    [[nodiscard]] unsigned int getOffset() const {
        return offset;
    }

    [[nodiscard]] unsigned int getSize() const {
        return size;
    }


private:
    std::shared_ptr<Scene> scene;

    std::string name;
    PUtilities::UniqueIdentifier uniqueIdentifier;

    std::vector<ShaderStage> shaderStages = {};
    unsigned int offset;
    unsigned int size;

    std::shared_ptr<PUtilities::RawDataContainer> rawDataContainer;

};
