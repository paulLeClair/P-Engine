//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../../Buffer.hpp"

class UntemplatedVertexBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;

        // TODO - any global vertex buffer data that's required (going to minimize it for now)
    };

    explicit UntemplatedVertexBuffer(const CreationInput &info);

    ~UntemplatedVertexBuffer() override = default;

    template<typename PointerType>
    void setRawDataAsArray(const std::vector<PointerType> &newRawData) {
        rawDataContainer->setRawDataAsArray<PointerType>(newRawData);
    }

    [[nodiscard]] bool isVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getNumberOfVertices() const {
        return numberOfVertices;
    }

    [[nodiscard]] void *getRawDataPointer() const {
        return rawDataContainer->getRawDataPointer<void>();
    }

    [[nodiscard]] virtual unsigned int getVertexSizeInBytes() const = 0;

    [[nodiscard]] virtual bool isPositionOnlyVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isPositionColorVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isPositionColorNormalVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isPositionColorNormalUVVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isPositionColorNormalUVTangentVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isPositionColorNormalUVTangentBitangentVertexBuffer() const {
        return false;
    }

protected:
    std::shared_ptr<RawDataContainer> rawDataContainer;

    unsigned long numberOfVertices;

};
