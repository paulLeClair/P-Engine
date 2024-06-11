//
// Created by paull on 2023-04-02.
//

#include "CharIndexBuffer.hpp"

using namespace pEngine::girEngine::scene;

UntemplatedCharIndexBuffer::UntemplatedCharIndexBuffer(const UntemplatedCharIndexBuffer::CreationInput &creationInput)
        : UntemplatedIndexBuffer(creationInput) {

}

std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation> UntemplatedCharIndexBuffer::bakeToGIR() {
    // TODO - provided we keep all this 
    return nullptr;
}
