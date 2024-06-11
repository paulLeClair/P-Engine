//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedIndexBuffer.hpp"

namespace pEngine::girEngine::scene {

    class UntemplatedIntIndexBuffer : public UntemplatedIndexBuffer {
    public:
        struct CreationInput : public UntemplatedIndexBuffer::CreationInput {
        };

        explicit UntemplatedIntIndexBuffer(const CreationInput &creationInput);

        ~UntemplatedIntIndexBuffer() override = default;

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override;

    };

}