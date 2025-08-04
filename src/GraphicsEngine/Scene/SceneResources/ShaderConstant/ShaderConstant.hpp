//
// Created by paull on 2023-03-06.
//

#pragma once

#include <vector>
#include <string>
#include <variant>
#include <memory>

//#include "../../../../lib/glm/glm.hpp"

#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../../utilities/ByteArray/ByteArray.hpp"

#include "../SceneResource.hpp"

#include "../../../GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp"

namespace pEngine::girEngine::scene {

/**
 * This should map to a push constant in Vulkan, or a cbuffer (etc) in DX12, and so on - basically constant memory
 * that is updated frequently by CPU and is faster for shaders to access (but there's limited space)
 */
    class ShaderConstant : public scene::Resource {
    public:
        // TODO - replace these weird vectors of enums with some kind of bitmask or something
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

        struct CreationInput : public Resource::CreationInput {
            std::vector<ShaderStage> shaderStages = {};
            unsigned int offset;
            unsigned int size;

            unsigned char *initializationDataPointer = nullptr;
            unsigned long initializationDataSizeInBytes = 0;


        };

        explicit ShaderConstant(const CreationInput &creationInput) : Resource(creationInput),
                                                                      shaderStages(
                                                                              creationInput.shaderStages),
                                                                      offset(creationInput.offset),
                                                                      size(creationInput.size) {
            rawDataContainer = util::ByteArray(
                    util::ByteArray::CreationInput{
                            getName(),
                            getUid(),
                            creationInput.initializationDataPointer,
                            creationInput.initializationDataSizeInBytes
                    });
        }


        ~ShaderConstant() override = default;

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

        [[nodiscard]] const util::ByteArray &getRawDataContainer() const {
            return rawDataContainer;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::ShaderConstantIR>(gir::ShaderConstantIR::CreationInput{
                    getName(),
                    getUid(),
                    gir::GIRSubtype::SHADER_CONSTANT,
                    rawDataContainer.getRawDataByteArray(),
                    static_cast<uint32_t>(rawDataContainer.getRawDataSizeInBytes())
            });
        }

    private:
        std::vector<ShaderStage> shaderStages = {};
        unsigned int offset;
        unsigned int size;

        util::ByteArray rawDataContainer;

    };

}