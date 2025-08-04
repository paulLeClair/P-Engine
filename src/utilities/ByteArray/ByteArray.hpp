//
// Created by paull on 2023-04-02.
//

#pragma once

#include <string>
#include <vector>

#include "../UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::util {
    class ByteArray {
    public:
        struct CreationInput {
            std::string name;
            UniqueIdentifier uniqueIdentifier;

            const uint8_t *rawData;
            const size_t rawDataSizeInBytes;

            // TODO - add configuration (eg controlling whether it's mutable or not, unsure of what else we'd need)
        };

        explicit ByteArray(const CreationInput &creationInput);

        explicit ByteArray(const std::string &name,
                           const UniqueIdentifier &uniqueIdentifier,
                           const uint8_t *rawData,
                           const size_t rawDataSizeInBytes)
            : ByteArray(CreationInput{
                    name,
                    uniqueIdentifier,
                    rawData,
                    rawDataSizeInBytes
                }
            ) {
        }

        // back to default copy ctor for now; change if needed
        ByteArray(const ByteArray &other) = default;

        ByteArray() : ByteArray(CreationInput{}) {
        }

        ~ByteArray() = default;

        // TODO -> rename this, it's an ugly name
        [[nodiscard]] const uint8_t *getRawDataByteArray() const {
            return getRawDataPointer<uint8_t>();
        }

        template<typename PointerType = void *>
        [[nodiscard]] const PointerType *getRawDataPointer() const {
            return reinterpret_cast<const PointerType *>(rawData.data());
        }

        [[nodiscard]] const std::vector<unsigned char> &getRawDataAsVectorOfBytes() const {
            return rawData;
        }

        template<typename ElementType>
        const std::vector<ElementType> &getRawDataAsVector() const {
            return (std::vector<ElementType> &) rawData;
        }

        void setRawData(const unsigned char *newRawData, unsigned long newRawDataSizeInBytes);

        template<typename PointerType>
        void setRawDataAsArray(const std::vector<PointerType> &newDataArray) {
            setRawData((uint8_t *) newDataArray.data(), newDataArray.size() * sizeof(PointerType));
        }

        [[nodiscard]] size_t getRawDataSizeInBytes() const;

        [[nodiscard]] size_t getNumberOfDataElements() const;

        void appendRawData(const void *rawDataToAppend, size_t appendedDataSizeInBytes) {
            if (!rawDataToAppend) {
                // TODO -> log!
                return;
            }

            rawDataSizeInBytes += appendedDataSizeInBytes;
            const auto tmp = rawData;
            rawData.clear();
            rawData.resize(rawDataSizeInBytes);
            std::memcpy(rawData.data(), tmp.data(), tmp.size() * sizeof(unsigned char));
            std::memcpy(rawData.data() + tmp.size() * sizeof(unsigned char), rawDataToAppend, appendedDataSizeInBytes);
        }

    private:
        std::string name;
        UniqueIdentifier uniqueIdentifier;

        /**
         * Byte array storing the actual data
         */
        std::vector<uint8_t> rawData;
        uint32_t rawDataSizeInBytes;
    };
} // PUtilities
