//
// Created by paull on 2022-12-28.
//

#include <cstring>
#include <stdexcept>
#include "VulkanBufferSuballocation.hpp"

using namespace pEngine::girEngine::backend::vulkan;

VulkanBufferSuballocation::VulkanBufferSuballocation(
    const CreationInput &creationInput) : uniqueIdentifier(creationInput.subsetUid),
                                          size(creationInput.subsetSize),
                                          globalBufferOffset(
                                              creationInput.globalBufferOffset),
                                          data(creationInput.subsetDataByteArray) {
}

size_t VulkanBufferSuballocation::getGlobalBufferOffset() const {
    return globalBufferOffset;
}

void VulkanBufferSuballocation::setGlobalBufferOffset(size_t offset) {
    globalBufferOffset = offset;
}

const pEngine::util::UniqueIdentifier &VulkanBufferSuballocation::getUniqueIdentifier() const {
    return uniqueIdentifier;
}

size_t VulkanBufferSuballocation::getSize() const {
    return size;
}

unsigned char *VulkanBufferSuballocation::getData() const {
    return (unsigned char *) data.data();
}

/**
 * \brief
 * \param newData
 * \param newDataSize
 */
void VulkanBufferSuballocation::setData(const unsigned char *newData, const size_t newDataSize) {
    try {
        if (newData == nullptr || newDataSize != size) {
            // TODO - log!
            throw std::runtime_error("Error in VulkanBufferSuballocation::setData() -> Invalid arguments provided!");
        }

        memcpy(data.data(), newData, size);
    } catch ([[maybe_unused]] std::exception &exception) {
        // TODO - log!
        throw std::runtime_error(
            "Error during VulkanBufferSuballocation->setData - unable to copy new data into subset! ");
    }
}
