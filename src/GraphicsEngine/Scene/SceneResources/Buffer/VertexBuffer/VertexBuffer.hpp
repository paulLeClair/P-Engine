//
// Created by paull on 2023-04-22.
//

#pragma once

#include "../Buffer.hpp"
#include "../../../PScene/PScene.hpp"
#include "VertexTypes/VertexTypes.hpp"

using namespace PGraphics;

template<typename VertexType>
class VertexBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<PScene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;

        std::vector<VertexType> initialVertexData = {};

        VertexTypeToken vertexTypeToken = VertexTypeToken::UNKNOWN;
    };

    explicit VertexBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
            std::dynamic_pointer_cast<Scene>(creationInput.parentScene),
            creationInput.name,
            creationInput.uniqueIdentifier,
            creationInput.updateCallback
    }) {
        vertexTypeToken = determineVertexTypeToken(creationInput.vertexTypeToken);
        if (vertexTypeToken == VertexTypeToken::UNKNOWN) {
            // TODO - better logging
            throw std::runtime_error("Error in VertexBuffer<>() - Unable to determine VertexTypeToken!");
        }

        rawDataContainer = std::make_shared<RawDataContainer>(RawDataContainer::CreationInput{
                this->getName() + "templatedVertexBuffer", // todo - incorporate the vertex type here
                this->getUniqueIdentifier(),
                nullptr,
                0
        });

        if (creationInput.initialVertexData.empty()) {
            numberOfVertices = 0;
        } else {
            rawDataContainer->setRawDataAsArray<VertexType>(creationInput.initialVertexData);
            numberOfVertices = creationInput.initialVertexData.size();
        }
    }

    [[nodiscard]] unsigned long getNumberOfVertices() const {
        return numberOfVertices;
    }

    [[nodiscard]] unsigned int getVertexSizeInBytes() const {
        return sizeof(VertexType);
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override {
        return numberOfVertices * sizeof(VertexType);
    }

    [[nodiscard]] void *getRawDataPointer() {
        return rawDataContainer->getRawDataPointer<void>();
    }

    VertexType *getVertexDataPointer() {
        return rawDataContainer->getRawDataPointer<VertexType>();
    }

    const std::vector<VertexType> &getVertexData() const {
        return rawDataContainer->getRawDataArray<VertexType>();
    }

    std::vector<VertexType> &getMutableVertexData() {
        return rawDataContainer->getRawDataArray<VertexType>();
    }

    void setVertexData(std::vector<VertexType> &newVertexData) {
        rawDataContainer->template setRawDataAsArray<VertexType>(newVertexData);
    }

    VertexType &getVertex(unsigned long vertexIndex) {
        if (vertexIndex > numberOfVertices) {
            // TODO - better logging
            throw std::runtime_error(
                    "Error in TemplatedVertexBuffer::getVertex() - Attempting to get out-of-bound vertex!");
        }
        return rawDataContainer->getRawDataPointer<VertexType>()[vertexIndex];
    }

    void setVertex(VertexType vertex, unsigned long index) {
        rawDataContainer->getRawDataPointer<VertexType>()[index] = vertex;
    }

    [[nodiscard]] VertexTypeToken getVertexTypeToken() const {
        return vertexTypeToken;
    }

private:
    VertexTypeToken vertexTypeToken = VertexTypeToken::UNKNOWN;

    std::shared_ptr<RawDataContainer> rawDataContainer;

    unsigned long numberOfVertices;

    VertexTypeToken determineVertexTypeToken(VertexTypeToken creationInputTypeToken) {
        if (creationInputTypeToken != VertexTypeToken::UNKNOWN) {
            return creationInputTypeToken;
        }

        bool isPositionOnlyVertexBuffer = std::is_same<VertexType, VertexTypes::PositionOnlyVertex>::value;
        if (isPositionOnlyVertexBuffer) {
            return VertexTypeToken::POSITION_ONLY;
        }

        bool isPositionColorVertexBuffer = std::is_same<VertexType, VertexTypes::PositionColorVertex>::value;
        if (isPositionColorVertexBuffer) {
            return VertexTypeToken::POSITION_COLOR;
        }

        bool isPositionColorNormalVertexBuffer = std::is_same<VertexType, VertexTypes::PositionColorNormalVertex>::value;
        if (isPositionColorNormalVertexBuffer) {
            return VertexTypeToken::POSITION_COLOR_NORMAL;
        }

        bool isPositionColorNormalUVVertexBuffer = std::is_same<VertexType, VertexTypes::PositionColorNormalUVVertex>::value;
        if (isPositionColorNormalUVVertexBuffer) {
            return VertexTypeToken::POSITION_COLOR_NORMAL_UV;
        }

        bool isPositionColorNormalUVTangentVertexBuffer = std::is_same<VertexType, VertexTypes::PositionColorNormalUVTangentVertex>::value;
        if (isPositionColorNormalUVTangentVertexBuffer) {
            return VertexTypeToken::POSITION_COLOR_NORMAL_UV_TANGENT;
        }

        bool isPositionColorNormalUVTangentBitangentVertexBuffer = std::is_same<VertexType, VertexTypes::PositionColorNormalUVTangentBitangentVertex>::value;
        if (isPositionColorNormalUVTangentBitangentVertexBuffer) {
            return VertexTypeToken::POSITION_COLOR_NORMAL_UV_TANGENT_BITANGENT;
        }

        return VertexTypeToken::UNKNOWN;
    }
};