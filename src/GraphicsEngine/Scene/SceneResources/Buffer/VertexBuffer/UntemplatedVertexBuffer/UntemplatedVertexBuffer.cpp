//
// Created by paull on 2023-04-02.
//

#include "UntemplatedVertexBuffer.hpp"

UntemplatedVertexBuffer::UntemplatedVertexBuffer(const UntemplatedVertexBuffer::CreationInput &info)
        : Buffer(Buffer::CreationInput{
        info.parentScene,
        info.name,
        info.uniqueIdentifier,
        info.updateCallback}) {
    numberOfVertices = 0;
}
