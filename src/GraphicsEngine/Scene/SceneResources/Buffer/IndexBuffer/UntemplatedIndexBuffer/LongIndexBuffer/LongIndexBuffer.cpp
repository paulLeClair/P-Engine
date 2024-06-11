//
// Created by paull on 2023-04-02.
//

#include "LongIndexBuffer.hpp"

using namespace pEngine::girEngine::scene;

UntemplatedLongIndexBuffer::UntemplatedLongIndexBuffer(const UntemplatedLongIndexBuffer::CreationInput &creationInput)
        : UntemplatedIndexBuffer(creationInput) {

}

std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> UntemplatedLongIndexBuffer::bakeToGIR() {
    // TODO (provided we keep all this lol)
    return nullptr;
}
