//
// Created by paull on 2023-04-02.
//

#include "ByteArray.hpp"

#include <stdexcept>

namespace pEngine::util {
    ByteArray::ByteArray(const ByteArray::CreationInput &creationInput)
            : name(creationInput.name),
              uniqueIdentifier(creationInput.uniqueIdentifier) {
        if (creationInput.rawData) {
            setRawData(creationInput.rawData, creationInput.rawDataSizeInBytes);
        } else {
            rawData = std::vector<unsigned char>(0);
            rawDataSizeInBytes = 0;
        }
    }

    void ByteArray::setRawData(const unsigned char *newRawData, unsigned long newRawDataSizeInBytes) {
        if (!newRawData) {
            // TODO - log or throw something here to indicate that the function was called with a nullptr argument
            return;
        }

        try {
            rawData = std::vector<uint8_t>(newRawDataSizeInBytes);
            std::memcpy(rawData.data(), newRawData, newRawDataSizeInBytes);
            rawDataSizeInBytes = newRawDataSizeInBytes;
        } catch (std::exception &e) {
            // TODO - log that an exception has been thrown etc
            throw std::runtime_error(
                    "Exception in PUtilities::RawDataContainer::setBufferData() - unable to std::copy new data!");
        }
    }

    size_t ByteArray::getRawDataSizeInBytes() const {
        return rawDataSizeInBytes;
    }

    size_t ByteArray::getNumberOfDataElements() const {
        return rawData.size();
    }
} // PUtilities
