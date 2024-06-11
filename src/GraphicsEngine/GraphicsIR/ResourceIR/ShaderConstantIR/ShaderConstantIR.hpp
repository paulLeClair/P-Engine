//
// Created by paull on 2023-09-27.
//

#pragma once

#include <memory>
#include "../../GraphicsIntermediateRepresentation.hpp"
#include "../../../../utilities/RawDataContainer/RawDataContainer.hpp"

namespace pEngine::girEngine::gir {
    class ShaderConstantIR : public GraphicsIntermediateRepresentation {
    public:
        // TODO - add any other necessary config for shader constants; not sure what we need right now

        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            const unsigned char *dataPointer = nullptr;
            const size_t numberOfBytesToCopy = 0;
        };

        explicit ShaderConstantIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              constantData(std::make_unique<util::RawDataContainer>(
                  util::RawDataContainer::CreationInput{
                      creationInput.name,
                      creationInput.uid,
                      const_cast<unsigned char *>(creationInput.dataPointer),
                      creationInput.numberOfBytesToCopy
                  })) {
        }

        ~ShaderConstantIR() override = default;

        [[nodiscard]] const std::unique_ptr<util::RawDataContainer> &getRawDataContainer() const {
            return constantData;
        }

        [[nodiscard]] unsigned getSizeInBytes() const {
            return sizeInBytes;
        }

        [[nodiscard]] unsigned getByteOffset() const {
            return offset;
        }

    private:
        unsigned offset;
        unsigned sizeInBytes;

        std::unique_ptr<util::RawDataContainer> constantData = nullptr;
    };
} // gir
