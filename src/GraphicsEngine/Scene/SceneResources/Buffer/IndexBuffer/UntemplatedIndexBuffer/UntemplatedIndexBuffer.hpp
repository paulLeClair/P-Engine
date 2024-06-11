//
// Created by paull on 2023-04-03.
//

#pragma once

#include "../../Buffer.hpp"

namespace pEngine::girEngine::scene {

    class UntemplatedIndexBuffer : public Buffer {
    public:
        struct CreationInput : public Buffer::CreationInput {
        };

        explicit UntemplatedIndexBuffer(const CreationInput &creationInput);

        ~UntemplatedIndexBuffer() override = default;

        virtual unsigned int getIndexTypeSizeInBytes() = 0;

        virtual unsigned int getNumberOfIndices() = 0;

        std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> bakeToGIR() override;
    };

}