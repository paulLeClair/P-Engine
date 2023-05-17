//
// Created by paull on 2023-04-02.
//

#pragma once

#include "../UntemplatedVertexBuffer.hpp"
#include "../../VertexTypes/VertexTypes.hpp"

class PositionColorNormalUVVertexBuffer : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit PositionColorNormalUVVertexBuffer(const CreationInput &creationInput);

    ~PositionColorNormalUVVertexBuffer() override = default;

    [[nodiscard]] bool isPositionColorNormalUVVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override;

    [[nodiscard]] unsigned int getVertexSizeInBytes() const override;

    VertexTypes::PositionColorNormalUVVertex *getPositionColorNormalUVVertexDataPointer();

    std::vector<VertexTypes::PositionColorNormalUVVertex> getPositionColorNormalUVVertexData();

    void setPositionColorNormalUVVertexData(std::vector<VertexTypes::PositionColorNormalUVVertex> &newVertexData);

private:


};
