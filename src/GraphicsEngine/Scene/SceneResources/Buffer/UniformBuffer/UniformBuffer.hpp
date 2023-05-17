#pragma once

#include "../Buffer.hpp"
#include "../../../PScene/PScene.hpp"

using namespace PGraphics;

class UniformBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<PScene> parentScene;
        std::string name;
        UniqueIdentifier uniqueIdentifier;
        std::function<void(const Buffer &)> updateCallback;

        unsigned char *initialDataPointer = nullptr;
        unsigned long initialDataSizeInBytes = 0;
    };

    explicit UniformBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
            std::dynamic_pointer_cast<Scene>(creationInput.parentScene),
            creationInput.name,
            creationInput.uniqueIdentifier,
            creationInput.updateCallback
    }) {
        if (!creationInput.initialDataSizeInBytes) {
            rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
                    creationInput.name + "UniformBuffer",
                    creationInput.uniqueIdentifier,
                    nullptr,
                    0
            });
        } else {
            rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
                    creationInput.name + "UniformBuffer",
                    creationInput.uniqueIdentifier,
                    creationInput.initialDataPointer,
                    creationInput.initialDataSizeInBytes
            });
        }
    }

    [[nodiscard]] bool isUniformBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override {
        return rawDataContainer->getRawDataSizeInBytes();
    }

    template<typename RawDataType>
    RawDataType *getRawDataAs() {
        return rawDataContainer->getRawDataPointer<RawDataType>();
    }

    template<typename RawDataType>
    void setRawData(RawDataType &newData) {
        setRawData(&newData);
    }

    template<typename RawDataType>
    void setRawData(RawDataType *newData) {
        rawDataContainer->setRawData(newData, sizeof(RawDataType));
    }

    template<typename RawDataType>
    void setRawDataAsArray(const std::vector<RawDataType> &newData) {
        rawDataContainer->setRawData(newData.data(), newData.size() * sizeof(RawDataType));
    }

    template<typename RawDataType>
    void setRawDataAs(const RawDataType *rawData, unsigned long long numberOfBytesToCopy) {
        rawDataContainer->setRawData((unsigned char *) rawData, numberOfBytesToCopy);
    }

private:
    std::shared_ptr<RawDataContainer> rawDataContainer;

};
