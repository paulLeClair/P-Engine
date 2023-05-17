//
// Created by paull on 2023-04-02.
//

#include "CharIndexBuffer.hpp"

CharIndexBufferTest::CharIndexBufferTest(const CharIndexBufferTest::CreationInput &creationInput)
        : UntemplatedIndexBuffer(
        UntemplatedIndexBuffer::CreationInput{
                creationInput.parentScene,
                creationInput.name,
                creationInput.uniqueIdentifier,
                creationInput.updateCallback}) {

}
