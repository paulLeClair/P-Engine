//
// Created by paull on 2023-09-27.
//

#pragma once

#include <memory>
#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../../../utilities/ByteArray/ByteArray.hpp"

namespace pEngine::girEngine::gir {
    class ShaderConstantIR : public GraphicsIntermediateRepresentation {
    public:
        // TODO - add any other necessary config for shader constants; not sure what we need right now

        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            const uint8_t *dataPointer = nullptr;
            const uint32_t numberOfBytesToCopy = 0;
        };

        explicit ShaderConstantIR(const CreationInput &creationInput)
                : GraphicsIntermediateRepresentation(creationInput),
                  constantData(util::ByteArray(
                          util::ByteArray::CreationInput{
                                  creationInput.name,
                                  creationInput.uid,
                                  const_cast<unsigned char *>(creationInput.dataPointer),
                                  creationInput.numberOfBytesToCopy
                          })) {
        }

        ~ShaderConstantIR() override = default;


        unsigned offset = 0;
        unsigned sizeInBytes = 0;

        util::ByteArray constantData = {};
    };
} // gir
