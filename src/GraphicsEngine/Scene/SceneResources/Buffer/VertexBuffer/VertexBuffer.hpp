//
// Created by paull on 2023-04-22.
//

#pragma once

#include <set>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include "../Buffer.hpp"
#include "VertexTypes/VertexTypes.hpp"
#include "../../../../../utilities/Hash/Hash.hpp"

#include "../../../../../lib/murmur3/MurmurHash3.hpp"

namespace pEngine::girEngine::scene {

    /**
     * DEPRECATED (and commented bcuz for some reason CMake wants to
     * include this even when I remove it from the source list)
     *
     */
//    class VertexBuffer : public Buffer {
//    public:
//        using PositionType = glm::vec3;
//        using NormalType = glm::vec3;
//        using TextureCoordinateType = glm::vec3;
//        using TangentType = glm::vec3;
//        using BitangentType = glm::vec3;
//        using ColorType = glm::vec4;
//        using AnimationBoneIndicesType = glm::uvec4;
//        using AnimationBoneWeightsType = glm::vec4;
//
//        // okay, so since I'm trying to disable the whole "really complicated vertex attribute descriptions" thing
//        // because it's ugly and basically is just raw vulkan, maybe we can avoid that by
//        // following the path I'm on now of hardcoding/forcing the user to follow certain conventions
//        // for what your vertex attributes look like (but other than that there aren't any restrictions);
//        // for the early stages of the engine I think that's totally fine, we can do another pass over all this
//        // stuff and broaden our horizons later
//        enum EnabledAttribute {
//            Position = 0,
//            Normal = 1,
//            UV = 2,
//            TangentAndBitangent = 3,
//            AnimationBoneIndicesAndWeights = 4,
//            Color = 5
//        };
//
//        struct CreationInput {
//            // note: this is not a subclass of Buffer::CreationInput
//            std::string name;
//            util::UniqueIdentifier uid;
//            BufferSubtype bufferType = BufferSubtype::UNKNOWN;
//
//            std::vector<EnabledAttribute> vertexDataAttributes = {EnabledAttribute::Position};
//
//            // TODO - evaluate whether passing in initial vertex data as a byte array is suitable (seems ok for now)
//            const std::vector<unsigned char> initialVertexData = {};
//            // the data will have to line up with the enabled attributes and they should be in the
//            // hardcoded ordering I'm putting in for now
//
//            // TODO - add another creation input struct and ctor overload that allows you
//            // to specify your data *by attribute* and then it will combine them into
//            // an interleaved (AoS) array for you (so you don't need to worry about the
//            // hardcoded order, you can just split your vertex data up)
//        };
//
//        explicit VertexBuffer(const CreationInput &creationInput)
//                : Buffer(Buffer::CreationInput{
//                creationInput.name,
//                creationInput.uid,
//                creationInput.bufferType
//        }) {
//            if (!creationInput.initialVertexData.empty()) {
//                bufferData.setRawData(creationInput.initialVertexData.data(),
//                                       creationInput.initialVertexData.size() * sizeof(unsigned char));
//            }
//            numberOfVertices = creationInput.initialVertexData.size() / sizeof(unsigned char);
//        }
//
//        [[nodiscard]] unsigned long getNumberOfVertices() const {
//            return numberOfVertices;
//        }
//
//        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
//            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
//                    getName(),
//                    getUid(),
//                    gir::GIRSubtype::BUFFER,
//                    gir::BufferIR::BufferUsage::VERTEX_BUFFER,
//                    getRawDataContainer().getRawDataByteArray(),
//                    getRawDataContainer().getRawDataSizeInBytes()
//            });
//        }
//
//        // TODO - write versions of this that are actually practical; maybe these will work after all tho
//        template<typename VertexType>
//        VertexType getVertex(unsigned long index) {
//            if (index >= numberOfVertices) {
//                throw std::runtime_error("Error in vertexBuffer<>::getVertex() - index is out of bounds!");
//            }
//            return bufferData.getRawDataAsVector<VertexType>()[index];
//        }
//
//        template<typename VertexType>
//        void setVertexAt(unsigned long index, VertexType newVertexValue) {
//            if (index >= numberOfVertices) {
//                throw std::runtime_error("Error in vertexBuffer<>::setVertexAt() - index is out of bounds!");
//            }
//            auto data = bufferData.getRawDataAsVector<VertexType>();
//            data[index] = newVertexValue;
//            bufferData.setRawDataAsArray<VertexType>(data);
//        }
//
//        template<typename VertexType>
//        const std::vector<VertexType> &getVertexData() {
//            return bufferData.getRawDataAsVector<VertexType>();
//        }
//
//        template<typename VertexType>
//        void setVertexData(const std::vector<VertexType> &newVertices) {
//            bufferData.setRawDataAsArray<VertexType>(newVertices);
//        }
//
//        /**
//         * TODO: rewrite all the update logic stuff
//         * @return
//         */
//        UpdateResult update() override {
//            return UpdateResult::FAILURE;
//        }
//
//        [[nodiscard]] const std::vector<EnabledAttribute> &getVertexAttributes() const {
//            return vertexDataAttributes;
//        }
//
////        util::Hash vertexAttributeHash;
//
//    private:
//        unsigned long numberOfVertices;
//
//        std::vector<EnabledAttribute> vertexDataAttributes = {EnabledAttribute::Position};
//
//        [[maybe_unused]]
//        static util::Hash
//        computeVertexAttributeHash(const std::vector<vertex::VertexAttributeDescription> &vertexAttributes) {
//            constexpr size_t MURMURHASH_SEED = 128u; // arbitrary
//
//            uint64_t tmp[2] = {0};
//            // idea: just murmur hash these?
//            // note: gotta make sure murmur hash is working fine for the optionals; i think it should be ok tho
//            MurmurHash3_x64_128(vertexAttributes.data(),
//                                vertexAttributes.size() * sizeof(vertex::VertexAttributeDescription), //NOLINT
//                                MURMURHASH_SEED,
//                                &tmp);
//            return tmp[0];
//        }
//    };
//}
