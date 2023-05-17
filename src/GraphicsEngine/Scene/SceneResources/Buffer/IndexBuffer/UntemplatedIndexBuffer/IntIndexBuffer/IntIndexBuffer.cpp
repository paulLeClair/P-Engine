//
// Created by paull on 2023-04-02.
//

#include "IntIndexBuffer.hpp"

IntIndexBuffer::IntIndexBuffer(const IntIndexBuffer::CreationInput &creationInput)
        : UntemplatedIndexBuffer(UntemplatedIndexBuffer::CreationInput
                                         {creationInput.parentScene,
                                          creationInput.name,
                                          creationInput.uniqueIdentifier,
                                          creationInput.updateCallback
                                         }) {

}
