//
// Created by paull on 2023-04-02.
//

#pragma once

#include <memory>
#include <functional>
#include "../../SceneResource.hpp"
#include "../Buffer.hpp"

namespace pEngine::girEngine::scene {
    /**
     * Storage buffers are slower than Uniform buffers but they support store operations
     * and they can be a lot bigger. \n\n
     *
     * Example use case: storing all objects
     * in a scene.
     */
    class StorageBuffer : public Buffer {
    public:
        struct CreationInput : public Buffer::CreationInput {
        };

        explicit StorageBuffer(const CreationInput &creationInput) : Buffer(creationInput) {
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
                getName(),
                getUid(),
                gir::GIRSubtype::BUFFER,
                {gir::BufferIR::BufferUsage::STORAGE_BUFFER},
                getRawDataContainer().getRawDataByteArray(),
                getRawDataContainer().getRawDataSizeInBytes()
            });
        }
    };
}
