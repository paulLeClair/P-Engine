#pragma once

#include "../Buffer.hpp"
#include "../../../PScene/PScene.hpp"
#include "IndexTypes/IndexTypes.hpp"

using namespace PGraphics;

template<typename IndexType>
class IndexBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<PScene> parentScene;
        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;
        std::function<void(const Buffer &)> updateCallback;

        std::vector<IndexType> initialIndexData = {};

        IndexTypeToken indexTypeToken = IndexTypeToken::UNKNOWN;
    };


    explicit IndexBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
            creationInput.parentScene,
            creationInput.name,
            creationInput.uniqueIdentifier,
            creationInput.updateCallback
    }) {
        indexTypeToken = getIndexTypeToken(creationInput.indexTypeToken);
        if (indexTypeToken == IndexTypeToken::UNKNOWN) {
            throw std::runtime_error("Error in IndexBuffer() - unknown index type");
        }

        rawDataContainer = std::make_shared<RawDataContainer>(
                RawDataContainer::CreationInput{
                        creationInput.name + "TemplatedIndexBuffer",
                        creationInput.uniqueIdentifier,
                        nullptr,
                        0
                }
        );
        numberOfIndices = 0;

        if (!creationInput.initialIndexData.empty()) {
            rawDataContainer->setRawDataAsArray(creationInput.initialIndexData);
            numberOfIndices = creationInput.initialIndexData.size();
        }
    }

    ~IndexBuffer() override = default;

    [[nodiscard]] unsigned long getNumberOfIndices() const {
        return numberOfIndices;
    }

    [[nodiscard]] unsigned int getIndexSizeInBytes() const {
        return sizeof(IndexType);
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override {
        return numberOfIndices * sizeof(IndexType);
    }

    [[nodiscard]] void *getRawDataPointer() {
        return rawDataContainer->getRawDataPointer<void>();
    }

    IndexType *getIndexDataPointer() {
        return rawDataContainer->getRawDataPointer<IndexType>();
    }

    const std::vector<IndexType> &getIndexData() const {
        return rawDataContainer->getRawDataArray<IndexType>();
    }

    std::vector<IndexType> &getMutableIndexData() {
        return rawDataContainer->getRawDataArray<IndexType>();
    }

    void setIndexData(std::vector<IndexType> &newVertexData) {
        rawDataContainer->setRawDataAsArray<IndexType>(newVertexData);
    }

    IndexType &getIndexAt(unsigned long arrayIndex) {
        return rawDataContainer->getRawDataPointer<IndexType>()[arrayIndex];
    }

    void setIndexAt(IndexType index, unsigned long arrayIndex) {
        rawDataContainer->getRawDataPointer<IndexType>()[arrayIndex] = index;
    }

    [[nodiscard]] IndexTypeToken getIndexTypeToken() const {
        return indexTypeToken;
    }

private:
    IndexTypeToken getIndexTypeToken(IndexTypeToken typeToken) {
        if (typeToken != IndexTypeToken::UNKNOWN) {
            return typeToken;
        }

        bool isUnsignedChar = std::is_same<IndexType, unsigned char>().value;
        if (isUnsignedChar) {
            return IndexTypeToken::UNSIGNED_CHAR;
        }

        bool isUnsignedInt = std::is_same<IndexType, unsigned int>().value;
        if (isUnsignedInt) {
            return IndexTypeToken::UNSIGNED_INT;
        }

        bool isUnsignedLong = std::is_same<IndexType, unsigned long>().value;
        if (isUnsignedLong) {
            return IndexTypeToken::UNSIGNED_LONG;
        }

        return IndexTypeToken::UNKNOWN;
    }

    std::shared_ptr<RawDataContainer> rawDataContainer;

    unsigned long numberOfIndices;

    IndexTypeToken indexTypeToken = IndexTypeToken::UNKNOWN;
};
