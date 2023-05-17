//
// Created by paull on 2023-04-02.
//

#include "LongIndexBuffer.hpp"

LongIndexBuffer::LongIndexBuffer(const LongIndexBuffer::CreationInput &creationInput) :
        UntemplatedIndexBuffer(UntemplatedIndexBuffer::CreationInput{
                creationInput.parentScene,
                creationInput.name,
                creationInput.uniqueIdentifier,
                creationInput.updateCallback
        }) {

}
