//
// Created by paull on 2022-12-28.
//

#include <stdexcept>
#include "VulkanBufferSuballocation.hpp"

using namespace PGraphics;

VulkanBufferSuballocation::VulkanBufferSuballocation(const VulkanBufferSuballocation::CreationInput &creationInput) :
        uniqueIdentifier(creationInput.subsetUid), size(creationInput.subsetSize),
        globalBufferOffset(creationInput.globalBufferOffset), data(creationInput.subsetDataByteArray) {

}

unsigned long long int VulkanBufferSuballocation::getGlobalBufferOffset() const {
    if (globalBufferOffset == -1) {
        throw std::runtime_error(
                "Error in VulkanBufferSuballocation::getGlobalBufferOffset -> Global buffer acquired without being set first!");
    }
    return globalBufferOffset;
}

void VulkanBufferSuballocation::setGlobalBufferOffset(unsigned long long int offset) {
    globalBufferOffset = offset;
}

const PUtilities::UniqueIdentifier &VulkanBufferSuballocation::getUniqueIdentifier() const {
    return uniqueIdentifier;
}

unsigned long long int VulkanBufferSuballocation::getSize() const {
    return size;
}

void *VulkanBufferSuballocation::getData() const {
    return (void *) data.data();
}

void VulkanBufferSuballocation::setData(unsigned int *newData) {
    try {
        memcpy(data.data(), newData, size);
    }
    catch (std::exception &exception) {
        // TODO - log!
        throw std::runtime_error(
                "Error during VulkanBufferSuballocation->setData - unable to copy new data into subset!");
    }

}