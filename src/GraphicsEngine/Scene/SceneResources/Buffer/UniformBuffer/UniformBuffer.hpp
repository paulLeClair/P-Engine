#pragma once

#include "../Buffer.hpp"
#include "../../../Scene.hpp"

namespace pEngine::girEngine::scene {
    class UniformBuffer : public Buffer {
    public:
        struct CreationInput : public Buffer::CreationInput {
            // whatever else is required for a uniform buffer
        };

        explicit UniformBuffer(const CreationInput &creationInput) : Buffer(creationInput) {
        }


        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::BufferIR>(
                gir::BufferIR::CreationInput{
                    getName(),
                    getUid(),
                    gir::GIRSubtype::BUFFER,
                    {gir::BufferIR::BufferUsage::UNIFORM_BUFFER},
                    getRawDataContainer().getRawDataByteArray(),
                    getRawDataContainer().getRawDataSizeInBytes()
                }
            );
        }
    };
}
