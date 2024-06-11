//
// Created by paull on 2023-04-02.
//

#pragma once

#include <string>
#include <vector>

#include "../UniqueIdentifier/UniqueIdentifier.hpp"

namespace pEngine::util {
    /**
     * The general idea here is that we just want to have a class which holds some data whose type we don't care about,
     * for the purpose of passing in our arbitrary data (with known size) and then having it be easily accessible
     * for std::copy()ing later; it will probably just provide an interface for obtaining different pointers to that data,
     * and also for overwriting/updating the data if needed
     *
     * I think the idea could be to have a "raw pointer in, raw pointer out" type of interface, where initially you
     * give it a raw (void *) pointer to the data and a number of bytes to copy, which it will store internally in a
     * way where the user doesn't have to care much about what it's really doing. Then it can provide a raw pointer
     * to its internal copy of the data, which can be used for copying the data into a buffer suballocation later.
     *
     */
    class RawDataContainer {
    public:
        struct CreationInput {
            std::string name;
            UniqueIdentifier uniqueIdentifier;

            unsigned char *rawData;
            size_t rawDataSizeInBytes;

            // TODO - add configuration (eg controlling whether it's mutable or not, unsure of what else we'd need)
        };

        explicit RawDataContainer(const CreationInput &creationInput);

        explicit RawDataContainer(const std::string &name,
                                  const UniqueIdentifier &uniqueIdentifier,
                                  unsigned char *rawData, // NOLINT
                                  const unsigned long rawDataSizeInBytes) : RawDataContainer(CreationInput{
                name, uniqueIdentifier, rawData, rawDataSizeInBytes
        }) {
        }

        // back to default copy ctor for now; change if needed
        RawDataContainer(const RawDataContainer &other) = default;

        ~RawDataContainer() = default;

        [[nodiscard]] const unsigned char *getRawDataByteArray() const {
            return getRawDataPointer<unsigned char>();
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
            setRawData((unsigned char *) newDataArray.data(), newDataArray.size() * sizeof(PointerType));
        }

        [[nodiscard]] size_t getRawDataSizeInBytes() const;

        [[nodiscard]] size_t getNumberOfDataElements() const;

    private:
        std::string name;
        UniqueIdentifier uniqueIdentifier;

        /**
         * Byte array storing the actual data
         */
        std::vector<unsigned char> rawData;
        size_t rawDataSizeInBytes;
    };
} // PUtilities
