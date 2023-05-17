//
// Created by paull on 2023-04-02.
//

#include "PositionOnlyVertexBuffer.hpp"

PositionOnlyVertexBuffer::PositionOnlyVertexBuffer(
        const PositionOnlyVertexBuffer::CreationInput &creationInput)
        : UntemplatedVertexBuffer(
        UntemplatedVertexBuffer::CreationInput{creationInput.parentScene,
                                               creationInput.name,
                                               creationInput.uniqueIdentifier,
                                               creationInput.updateCallback}) {
    rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
            creationInput.name,
            creationInput.uniqueIdentifier,
            (unsigned char *) creationInput.positionOnlyVertexData.data(),
            static_cast<unsigned long>(creationInput.positionOnlyVertexData.size() *
                                       sizeof(VertexTypes::PositionOnlyVertex))
    });
    rawDataContainer->setRawDataAsArray<VertexTypes::PositionOnlyVertex>(creationInput.positionOnlyVertexData);
}

unsigned int PositionOnlyVertexBuffer::getVertexSizeInBytes() const {
    return sizeof(VertexTypes::PositionOnlyVertex);
}

unsigned long PositionOnlyVertexBuffer::getSizeInBytes() const {
    return rawDataContainer->getRawDataSizeInBytes();
}

VertexTypes::PositionOnlyVertex *PositionOnlyVertexBuffer::getPositionOnlyVertexDataPointer() {
    return rawDataContainer->getRawDataPointer<VertexTypes::PositionOnlyVertex>();
}

void PositionOnlyVertexBuffer::setPositionOnlyVertexData(std::vector<VertexTypes::PositionOnlyVertex> &newVertexData) {
    rawDataContainer->setRawData((unsigned char *) newVertexData.data(),
                                 newVertexData.size() * sizeof(VertexTypes::PositionOnlyVertex));
}

std::vector<VertexTypes::PositionOnlyVertex> PositionOnlyVertexBuffer::getPositionOnlyVertexData() {
    std::vector<VertexTypes::PositionOnlyVertex> vertexData(numberOfVertices);

    for (unsigned int index = 0; index < numberOfVertices; index++) {
        vertexData[index] = rawDataContainer->getRawDataPointer<VertexTypes::PositionOnlyVertex>()[index];
    }

    return vertexData;
}
