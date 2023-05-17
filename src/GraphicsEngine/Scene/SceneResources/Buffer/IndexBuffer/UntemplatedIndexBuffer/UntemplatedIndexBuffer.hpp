//
// Created by paull on 2023-04-03.
//

#pragma once

#include "../../Buffer.hpp"

class UntemplatedIndexBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit UntemplatedIndexBuffer(const CreationInput &creationInput);

    ~UntemplatedIndexBuffer() override = default;

    [[nodiscard]] virtual std::shared_ptr<Buffer> toBuffer(const std::shared_ptr<Scene> &parentScene) const = 0;

    virtual unsigned int getIndexTypeSizeInBytes() = 0;

    virtual unsigned int getNumberOfIndices() = 0;

    [[nodiscard]] bool isIndexBuffer() const override {
        return true;
    }

    [[nodiscard]] virtual bool isCharIndices() const {
        return false;
    }

    [[nodiscard]] virtual bool isIntIndices() const {
        return false;
    }

    [[nodiscard]] virtual bool isLongIndices() const {
        return false;
    }
};
