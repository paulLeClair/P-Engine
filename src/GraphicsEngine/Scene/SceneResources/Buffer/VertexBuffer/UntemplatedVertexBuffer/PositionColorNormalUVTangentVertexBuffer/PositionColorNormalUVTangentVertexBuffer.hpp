//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedVertexBuffer.hpp"

class PositionColorNormalUVTangentVertexBufferTest : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit PositionColorNormalUVTangentVertexBufferTest(const CreationInput &creationInput);

    [[nodiscard]] bool isPositionColorNormalUVTangentVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override;

    [[nodiscard]] unsigned int getVertexSizeInBytes() const override;


};
