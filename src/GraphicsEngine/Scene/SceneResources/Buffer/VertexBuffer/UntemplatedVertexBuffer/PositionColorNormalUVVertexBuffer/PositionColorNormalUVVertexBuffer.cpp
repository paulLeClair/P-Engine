//
// Created by paull on 2023-04-02.
//

#include "PositionColorNormalUVVertexBuffer.hpp"

PositionColorNormalUVVertexBuffer::PositionColorNormalUVVertexBuffer(
        const PositionColorNormalUVVertexBuffer::CreationInput &creationInput)
        : UntemplatedVertexBuffer(UntemplatedVertexBuffer::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.updateCallback}) {

}

unsigned int PositionColorNormalUVVertexBuffer::getVertexSizeInBytes() const {
    return numberOfVertices * sizeof(VertexTypes::PositionColorNormalUVVertex);
}

VertexTypes::PositionColorNormalUVVertex *
PositionColorNormalUVVertexBuffer::getPositionColorNormalUVVertexDataPointer() {
    return rawDataContainer->getRawDataPointer<VertexTypes::PositionColorNormalUVVertex>();
}

unsigned long PositionColorNormalUVVertexBuffer::getSizeInBytes() const {
    return numberOfVertices * sizeof(VertexTypes::PositionColorNormalUVVertex);
}

std::vector<VertexTypes::PositionColorNormalUVVertex>
PositionColorNormalUVVertexBuffer::getPositionColorNormalUVVertexData() {
    std::vector<VertexTypes::PositionColorNormalUVVertex> vertexData(numberOfVertices);

    for (unsigned int index = 0; index < numberOfVertices; index++) {
        vertexData[index] = rawDataContainer->getRawDataPointer<VertexTypes::PositionColorNormalUVVertex>()[index];
    }

    return vertexData;
}

void PositionColorNormalUVVertexBuffer::setPositionColorNormalUVVertexData(
        std::vector<VertexTypes::PositionColorNormalUVVertex> &newVertexData) {
    rawDataContainer->setRawDataAsArray<VertexTypes::PositionColorNormalUVVertex>(newVertexData);
}


