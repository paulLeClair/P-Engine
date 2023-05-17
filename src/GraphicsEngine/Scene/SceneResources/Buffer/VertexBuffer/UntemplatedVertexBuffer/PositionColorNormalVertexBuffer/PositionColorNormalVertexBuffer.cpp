//
// Created by paull on 2023-04-02.
//

#include "PositionColorNormalVertexBuffer.hpp"

PositionColorNormalVertexBuffer::PositionColorNormalVertexBuffer(
        const PositionColorNormalVertexBuffer::CreationInput &creationInput)
        : UntemplatedVertexBuffer(UntemplatedVertexBuffer::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.updateCallback}) {

}

VertexTypes::PositionColorNormalVertex *PositionColorNormalVertexBuffer::getPositionColorNormalVertexDataPointer() {
    return rawDataContainer->getRawDataPointer<VertexTypes::PositionColorNormalVertex>();
}

void
PositionColorNormalVertexBuffer::setPositionColorNormalVertexData(
        std::vector<VertexTypes::PositionColorNormalVertex> &newVertexData) {
    rawDataContainer->setRawData((unsigned char *) newVertexData.data(),
                                 newVertexData.size() * sizeof(VertexTypes::PositionColorNormalVertex));
}

std::vector<VertexTypes::PositionColorNormalVertex>
PositionColorNormalVertexBuffer::getPositionColorNormalVertexData() {
    std::vector<VertexTypes::PositionColorNormalVertex> vertexData(numberOfVertices);

    for (unsigned int index = 0; index < numberOfVertices; index++) {
        vertexData[index] = rawDataContainer->getRawDataPointer<VertexTypes::PositionColorNormalVertex>()[index];
    }

    return vertexData;
}

unsigned long PositionColorNormalVertexBuffer::getSizeInBytes() const {
    return numberOfVertices * getVertexSizeInBytes();
}

unsigned int PositionColorNormalVertexBuffer::getVertexSizeInBytes() const {
    return sizeof(VertexTypes::PositionColorNormalVertex);
}
