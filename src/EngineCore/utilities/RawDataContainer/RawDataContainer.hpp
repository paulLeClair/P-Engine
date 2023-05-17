//
// Created by paull on 2023-04-02.
//

#pragma once

#include <string>
#include <vector>

#include "../UniqueIdentifier/UniqueIdentifier.hpp"

namespace PUtilities {

    /**
     * TODO - this will be what I try to wrap up arbitrary data for buffers/etc
     *
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
            PUtilities::UniqueIdentifier uniqueIdentifier;

            unsigned char *rawData;
            unsigned long rawDataSizeInBytes;

            // TODO - add configuration flags (eg whether it's mutable or not, unsure of what else we'd need)
        };

        explicit RawDataContainer(const CreationInput &creationInput);

        ~RawDataContainer() = default;

        template<typename PointerType>
        [[nodiscard]] PointerType *getRawDataPointer() {
            return reinterpret_cast<PointerType *>(rawData.data());
        }

        [[nodiscard]] const std::vector<unsigned char> &getRawDataCharArray() const {
            return rawData;
        }

        template<typename ElementType>
        std::vector<ElementType> &getRawDataArray() {
            return (std::vector<ElementType> &) rawData;
        }

        void setRawData(unsigned char *newRawData, unsigned long newRawDataSizeInBytes);

        template<typename PointerType>
        void setRawDataAsArray(const std::vector<PointerType> &newDataArray) {
            setRawData((unsigned char *) newDataArray.data(), newDataArray.size() * sizeof(PointerType));
        }

        [[nodiscard]] unsigned long getRawDataSizeInBytes() const;

    private:
        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        /**
         * Byte array storing the actual data
         */
        std::vector<unsigned char> rawData;
        unsigned long rawDataSizeInBytes;
    };

} // PUtilities
