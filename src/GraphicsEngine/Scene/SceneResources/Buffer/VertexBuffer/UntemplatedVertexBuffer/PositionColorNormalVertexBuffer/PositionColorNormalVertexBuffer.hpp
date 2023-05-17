#pragma once

#include "../UntemplatedVertexBuffer.hpp"
#include "../../VertexTypes/VertexTypes.hpp"

class PositionColorNormalVertexBuffer : public UntemplatedVertexBuffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit PositionColorNormalVertexBuffer(const CreationInput &creationInput);

    ~PositionColorNormalVertexBuffer() override = default;

    [[nodiscard]] bool isPositionColorNormalVertexBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override;

    [[nodiscard]] unsigned int getVertexSizeInBytes() const override;

    VertexTypes::PositionColorNormalVertex *getPositionColorNormalVertexDataPointer();

    std::vector<VertexTypes::PositionColorNormalVertex> getPositionColorNormalVertexData();

    void setPositionColorNormalVertexData(std::vector<VertexTypes::PositionColorNormalVertex> &newVertexData);

};
