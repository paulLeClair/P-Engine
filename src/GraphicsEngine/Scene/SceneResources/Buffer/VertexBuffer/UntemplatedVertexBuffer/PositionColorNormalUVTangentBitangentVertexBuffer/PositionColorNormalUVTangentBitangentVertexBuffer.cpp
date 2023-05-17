//
// Created by paull on 2023-04-02.
//

#include "PositionColorNormalUVTangentBitangentVertexBuffer.hpp"

PositionColorNormalUVTangentBitangentVertexBuffer::PositionColorNormalUVTangentBitangentVertexBuffer(
        const PositionColorNormalUVTangentBitangentVertexBuffer::CreationInput &creationInput)
        : UntemplatedVertexBuffer(UntemplatedVertexBuffer::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.updateCallback}) {

}

unsigned long PositionColorNormalUVTangentBitangentVertexBuffer::getSizeInBytes() const {
    return 0;
}

unsigned int PositionColorNormalUVTangentBitangentVertexBuffer::getVertexSizeInBytes() const {
    return 0;
}

