//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedIndexBuffer.hpp"

namespace pEngine::girEngine::scene {

    class UntemplatedCharIndexBuffer : public UntemplatedIndexBuffer {
    public:
        struct CreationInput : public UntemplatedIndexBuffer::CreationInput {
        };

        explicit UntemplatedCharIndexBuffer(const CreationInput &creationInput);

        ~UntemplatedCharIndexBuffer() override = default;

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override;
    };

}