//
// Created by paull on 2023-04-02.
//

#include "PositionColorNormalUVTangentVertexBuffer.hpp"
#include "../../VertexTypes/VertexTypes.hpp"

PositionColorNormalUVTangentVertexBufferTest::PositionColorNormalUVTangentVertexBufferTest(
        const PositionColorNormalUVTangentVertexBufferTest::CreationInput &creationInput)
        : UntemplatedVertexBuffer(UntemplatedVertexBuffer::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.updateCallback}) {

}

unsigned long PositionColorNormalUVTangentVertexBufferTest::getSizeInBytes() const {
    return numberOfVertices * sizeof(VertexTypes::PositionColorNormalUVTangentVertex);
}

unsigned int PositionColorNormalUVTangentVertexBufferTest::getVertexSizeInBytes() const {
    return sizeof(VertexTypes::PositionColorNormalUVTangentVertex);
}




