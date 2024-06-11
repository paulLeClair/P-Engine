//
// Created by paull on 2024-05-01.
//

#pragma once

#include "../../Buffer.hpp"
#include "../VertexTypes/VertexTypes.hpp"
#include "../../../../../../utilities/Hash/Hash.hpp"

#include "../../../../../../lib/murmur3/MurmurHash3.hpp"

namespace pEngine::girEngine::scene {

    /**
     * Leaving this here for now; not adding it to the project, will probably be deleting this class at some point
     * @tparam VertexType
     */
    template<typename VertexType>
    class VertexBuffer : public Buffer {
    public:
        struct CreationInput {
            // note: this is not a subclass of Buffer::CreationInput
            std::string name;
            util::UniqueIdentifier uid;
            BufferSubtype bufferType = BufferSubtype::UNKNOWN;

            const std::vector<VertexType> initialVertexData = {};

            const std::vector<vertex::VertexAttributeDescription> vertexAttributes = {};
        };

        explicit VertexBuffer(const CreationInput &creationInput)
                : vertexAttributes(creationInput.vertexAttributes),
                  vertexAttributeHash(computeVertexAttributeHash(creationInput.vertexAttributes)),
                  Buffer(Buffer::CreationInput{
                          creationInput.name,
                          creationInput.uid,
                          creationInput.bufferType
                  }) {
            if (!creationInput.initialVertexData.empty()) {
                setBufferDataAsArray<VertexType>(creationInput.initialVertexData);
            }
            numberOfVertices = creationInput.initialVertexData.size();
        }

        [[nodiscard]] unsigned long getNumberOfVertices() const {
            return numberOfVertices;
        }

        [[nodiscard]] static unsigned int getVertexSizeInBytes() {
            return sizeof(VertexType);
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
                    getName(),
                    getUid(),
                    gir::GIRSubtype::BUFFER,
                    {gir::BufferIR::BufferUsage::VERTEX_BUFFER},
                    getRawDataContainer().getRawDataByteArray(),
                    getRawDataContainer().getRawDataSizeInBytes()
            });
        }

        VertexType getVertex(unsigned long index) {
            if (index >= numberOfVertices) {
                throw std::runtime_error("Error in vertexBuffer<>::getVertex() - index is out of bounds!");
            }
            return bufferData->getRawDataAsVector<VertexType>()[index];
        }

        void setVertexAt(unsigned long index, VertexType newVertexValue) {
            if (index >= numberOfVertices) {
                throw std::runtime_error("Error in vertexBuffer<>::setVertexAt() - index is out of bounds!");
            }
            auto data = bufferData->getRawDataAsVector<VertexType>();
            data[index] = newVertexValue;
            bufferData->setRawDataAsArray<VertexType>(data);
        }

        const std::vector<VertexType> &getVertexData() {
            return bufferData->getRawDataAsVector<VertexType>();
        }

        void setVertexData(const std::vector<VertexType> &newVertices) {
            bufferData->setRawDataAsArray<VertexType>(newVertices);
        }

        /**
         * TODO: rewrite all the update logic stuff
         * @return
         */
        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        [[nodiscard]] const std::vector<vertex::VertexAttributeDescription> &getVertexAttributes() const {
            return vertexAttributes;
        }

        [[nodiscard]] Hash getVertexAttributeHash() const {
            return vertexAttributeHash;
        }

    private:
        unsigned long numberOfVertices;

        // 04-23 - coming back to this, we're gonna have to have each vertex buffer maintain its own attributes
        std::vector<vertex::VertexAttributeDescription> vertexAttributes = {};

        util::Hash vertexAttributeHash;

        static util::Hash
        computeVertexAttributeHash(const std::vector<vertex::VertexAttributeDescription> &vertexAttributes) {
            constexpr size_t MURMURHASH_SEED = 128u; // arbitrary

            uint64_t tmp[2] = {0};
            // idea: just murmur hash these?
            // note: gotta make sure murmur hash is working fine for the optionals; i think it should be ok tho
            MurmurHash3_x64_128(vertexAttributes.data(),
                                vertexAttributes.size() * sizeof(vertex::VertexAttributeDescription), //NOLINT
                                MURMURHASH_SEED,
                                &tmp);
            return tmp[0];
        }
    };

}