//
// Created by paull on 2023-04-22.
//

#pragma once

#include "../Buffer.hpp"
#include "VertexTypes/VertexTypes.hpp"
#include "../../../../../utilities/Hash/Hash.hpp"

#include "../../../../../lib/murmur3/MurmurHash3.hpp"

namespace pEngine::girEngine::scene {

    /**
     * OKAY - 04-28 -> returning to this, I am now potentially running into an issue with using template classes;
     * accessing certain member functions becomes tricky because you cant pointer cast to a particular type if you
     * don't know what the vertex type will be. To make it work the way I want with templates is probably too much
     * headache and misery to be worth doing.
     *
     * Instead, my main idea:
     * 1. remove the class template and just have the buffer provide some template arguments for situations where
     * you do have a vertex type that you have in mind; this isn't strictly necessary I think with the new design
     * tho so these methods might not see much use
     * 2. have the vertex attribute information (and maybe some new fields if req'd) accomplish the task of describing
     * what's inside the buffer without needing to use templates. Since this is just raw data with format info, we
     * don't really need to do much to it other than get it ready to be packaged up for Vulkan
     *
     *
     */

    class VertexBuffer : public Buffer {
    public:
        struct CreationInput {
            // note: this is not a subclass of Buffer::CreationInput
            std::string name;
            util::UniqueIdentifier uid;
            BufferSubtype bufferType = BufferSubtype::UNKNOWN;

            // TODO - evaluate whether passing in initial vertex data as a byte array is suitable (seems ok for now)
            const std::vector<unsigned char> initialVertexData = {};

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
                bufferData->setRawData(creationInput.initialVertexData.data(),
                                       creationInput.initialVertexData.size() * sizeof(unsigned char));
            }
            numberOfVertices = creationInput.initialVertexData.size();
        }

        [[nodiscard]] unsigned long getNumberOfVertices() const {
            return numberOfVertices;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
                    getName(),
                    getUid(),
                    gir::GIRSubtype::BUFFER,
                    gir::BufferIR::BufferUsage::VERTEX_BUFFER,
                    getRawDataContainer().getRawDataByteArray(),
                    getRawDataContainer().getRawDataSizeInBytes()
            });
        }

        // TODO - write versions of this that are actually practical; maybe these will work after all tho
        template<typename VertexType>
        VertexType getVertex(unsigned long index) {
            if (index >= numberOfVertices) {
                throw std::runtime_error("Error in vertexBuffer<>::getVertex() - index is out of bounds!");
            }
            return bufferData->getRawDataAsVector<VertexType>()[index];
        }

        template<typename VertexType>
        void setVertexAt(unsigned long index, VertexType newVertexValue) {
            if (index >= numberOfVertices) {
                throw std::runtime_error("Error in vertexBuffer<>::setVertexAt() - index is out of bounds!");
            }
            auto data = bufferData->getRawDataAsVector<VertexType>();
            data[index] = newVertexValue;
            bufferData->setRawDataAsArray<VertexType>(data);
        }

        template<typename VertexType>
        const std::vector<VertexType> &getVertexData() {
            return bufferData->getRawDataAsVector<VertexType>();
        }

        template<typename VertexType>
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

        util::Hash vertexAttributeHash;

    private:
        unsigned long numberOfVertices;

        // 04-23 - coming back to this, we're gonna have to have each vertex buffer maintain its own attributes
        std::vector<vertex::VertexAttributeDescription> vertexAttributes = {};


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
