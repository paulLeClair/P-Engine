//
// Created by paull on 2023-04-02.
//

#include "PositionColorVertexBuffer.hpp"

PositionColorVertexBuffer::PositionColorVertexBuffer(const PositionColorVertexBuffer::CreationInput &creationInput)
        : UntemplatedVertexBuffer(UntemplatedVertexBuffer::CreationInput{
        creationInput.parentScene,
        creationInput.name,
        creationInput.uniqueIdentifier,
        creationInput.updateCallback}) {

}

unsigned int PositionColorVertexBuffer::getVertexSizeInBytes() const {
    return sizeof(VertexTypes::PositionColorVertex);
}

unsigned long PositionColorVertexBuffer::getSizeInBytes() const {
    return numberOfVertices * getVertexSizeInBytes();
}

VertexTypes::PositionColorVertex *PositionColorVertexBuffer::getPositionColorVertexDataPointer() {
    return rawDataContainer->getRawDataPointer<VertexTypes::PositionColorVertex>();
}

void
PositionColorVertexBuffer::setPositionColorVertexData(std::vector<VertexTypes::PositionColorVertex> &newVertexData) {
    rawDataContainer->setRawData((unsigned char *) newVertexData.data(),
                                 newVertexData.size() * sizeof(VertexTypes::PositionColorVertex));
}

std::vector<VertexTypes::PositionColorVertex> PositionColorVertexBuffer::getPositionColorVertexData() {
    std::vector<VertexTypes::PositionColorVertex> vertexData(numberOfVertices);

    for (unsigned int index = 0; index < numberOfVertices; index++) {
        vertexData[index] = rawDataContainer->getRawDataPointer<VertexTypes::PositionColorVertex>()[index];
    }

    return vertexData;
}
