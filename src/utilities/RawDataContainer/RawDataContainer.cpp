//
// Created by paull on 2023-04-02.
//

#include "RawDataContainer.hpp"

#include <stdexcept>

namespace pEngine::util {
    RawDataContainer::RawDataContainer(const RawDataContainer::CreationInput &creationInput)
            : name(creationInput.name),
              uniqueIdentifier(creationInput.uniqueIdentifier) {
        if (name.empty()) {
            throw std::runtime_error("Runtime error in RawDataContainer() - Improper name provided!");
        }

        if (creationInput.rawData) {
            setRawData(creationInput.rawData, creationInput.rawDataSizeInBytes);
        } else {
            rawData = std::vector<unsigned char>(0);
            rawDataSizeInBytes = 0;
        }
    }

    void RawDataContainer::setRawData(const unsigned char *newRawData, unsigned long newRawDataSizeInBytes) {
        if (!newRawData) {
            // TODO - log or throw something here to indicate that the function was called with a nullptr argument
            return;
        }

        try {
            rawData = std::vector<unsigned char>(newRawDataSizeInBytes);
            std::copy(newRawData, newRawData + newRawDataSizeInBytes,
                      rawData.data());
            rawDataSizeInBytes = newRawDataSizeInBytes;
        } catch (std::exception &e) {
            // TODO - log that an exception has been thrown etc
            throw std::runtime_error(
                    "Exception in PUtilities::RawDataContainer::setBufferData() - unable to std::copy new data!");
        }
    }

    size_t RawDataContainer::getRawDataSizeInBytes() const {
        return rawDataSizeInBytes;
    }

    size_t RawDataContainer::getNumberOfDataElements() const {
        return rawData.size();
    }
} // PUtilities
