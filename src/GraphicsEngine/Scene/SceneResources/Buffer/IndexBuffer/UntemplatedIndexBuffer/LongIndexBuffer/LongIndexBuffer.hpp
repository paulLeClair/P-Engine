//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedIndexBuffer.hpp"

namespace pEngine::girEngine::scene {

    class UntemplatedLongIndexBuffer : public UntemplatedIndexBuffer {
    public:
        struct CreationInput : public UntemplatedIndexBuffer::CreationInput {

        };

        explicit UntemplatedLongIndexBuffer(const CreationInput &creationInput);

        ~UntemplatedLongIndexBuffer() override = default;

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override;
    };

}