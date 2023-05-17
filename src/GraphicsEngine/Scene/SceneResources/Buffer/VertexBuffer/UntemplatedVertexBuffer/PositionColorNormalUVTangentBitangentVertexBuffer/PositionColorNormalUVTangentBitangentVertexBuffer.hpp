//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedVertexBuffer.hpp"

class PositionColorNormalUVTangentBitangentVertexBuffer : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit PositionColorNormalUVTangentBitangentVertexBuffer(const CreationInput &creationInput);

    ~PositionColorNormalUVTangentBitangentVertexBuffer() override = default;

    [[nodiscard]] bool isPositionColorNormalUVTangentBitangentVertexBuffer() const override {
        return true;
    }

    unsigned long getSizeInBytes() const override;

    unsigned int getVertexSizeInBytes() const override;
};
