//
// Created by paull on 2023-04-03.
//

#include "UntemplatedIndexBuffer.hpp"

UntemplatedIndexBuffer::UntemplatedIndexBuffer(const UntemplatedIndexBuffer::CreationInput &creationInput) : Buffer(
        Buffer::CreationInput{
                creationInput.parentScene,
                creationInput.name,
                creationInput.uniqueIdentifier,
                creationInput.updateCallback}) {

}