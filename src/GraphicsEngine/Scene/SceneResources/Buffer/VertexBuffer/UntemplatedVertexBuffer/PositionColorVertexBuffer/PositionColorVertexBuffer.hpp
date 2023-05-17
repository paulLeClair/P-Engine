//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedVertexBuffer.hpp"
#include "../../VertexTypes/VertexTypes.hpp"

class PositionColorVertexBuffer : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit PositionColorVertexBuffer(const CreationInput &creationInput);

    ~PositionColorVertexBuffer() override = default;

    [[nodiscard]] bool isPositionColorVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override;

    [[nodiscard]] unsigned int getVertexSizeInBytes() const override;

    VertexTypes::PositionColorVertex *getPositionColorVertexDataPointer();

    std::vector<VertexTypes::PositionColorVertex> getPositionColorVertexData();

    void setPositionColorVertexData(std::vector<VertexTypes::PositionColorVertex> &newVertexData);

};
