//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedVertexBuffer.hpp"
#include "../../VertexTypes/VertexTypes.hpp"

using namespace PUtilities;

class PositionOnlyVertexBuffer : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;

        std::vector<VertexTypes::PositionOnlyVertex> positionOnlyVertexData = {};
    };

    explicit PositionOnlyVertexBuffer(const CreationInput &creationInput);

    ~PositionOnlyVertexBuffer() override = default;

    [[nodiscard]] bool isPositionOnlyVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned int getVertexSizeInBytes() const override;

    [[nodiscard]] unsigned long getSizeInBytes() const override;

    VertexTypes::PositionOnlyVertex *getPositionOnlyVertexDataPointer();

    std::vector<VertexTypes::PositionOnlyVertex> getPositionOnlyVertexData();

    void setPositionOnlyVertexData(std::vector<VertexTypes::PositionOnlyVertex> &newVertexData);

};
