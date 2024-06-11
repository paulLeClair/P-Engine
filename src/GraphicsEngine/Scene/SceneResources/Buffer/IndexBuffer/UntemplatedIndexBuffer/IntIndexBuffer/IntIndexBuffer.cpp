//
// Created by paull on 2023-04-02.
//

#include "IntIndexBuffer.hpp"

using namespace pEngine::girEngine::scene;

UntemplatedIntIndexBuffer::UntemplatedIntIndexBuffer(const UntemplatedIntIndexBuffer::CreationInput &creationInput)
        : UntemplatedIndexBuffer(creationInput) {

}

std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> UntemplatedIntIndexBuffer::bakeToGIR() {
    // TODO (provided we keep all this)
    return nullptr;
}
