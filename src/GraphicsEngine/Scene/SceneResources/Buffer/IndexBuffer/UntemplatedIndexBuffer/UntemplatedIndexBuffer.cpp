//
// Created by paull on 2023-04-03.
//

#include "UntemplatedIndexBuffer.hpp"

using namespace pEngine::girEngine::scene;

UntemplatedIndexBuffer::UntemplatedIndexBuffer(const UntemplatedIndexBuffer::CreationInput &creationInput)
        : Buffer(creationInput) {

}

std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> UntemplatedIndexBuffer::bakeToGIR() {
    // TODO (provided we keep this class...)
    return nullptr;
}
