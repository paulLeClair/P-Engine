#pragma once

#include "../Buffer.hpp"
#include "IndexTypes/IndexTypes.hpp"

namespace pEngine::girEngine::scene {

//    class IndexBuffer : public Buffer {
//    public:
//        struct CreationInput {
//            // note: this is not a subclass of Buffer::CreationInput
//            std::string name;
//            util::UniqueIdentifier uid;
//            BufferSubtype bufferType = BufferSubtype::UNKNOWN;
//
//            const std::vector<unsigned char> initialIndexData = {};
//        };
//
//        explicit IndexBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
//                creationInput.name,
//                creationInput.uid,
//                creationInput.bufferType
//        }) {
//            if (!creationInput.initialIndexData.empty()) {
//                bufferData.setRawData(creationInput.initialIndexData.data(),
//                                      creationInput.initialIndexData.size() * sizeof(unsigned char));
//            }
//            numberOfIndices = creationInput.initialIndexData.size();
//        }
//
//        ~IndexBuffer() override = default;
//
//        [[nodiscard]] unsigned long getNumberOfIndices() const {
//            return numberOfIndices;
//        }
//
//        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
//            return std::make_shared<gir::BufferIR>(gir::BufferIR::CreationInput{
//                    getName(),
//                    getUid(),
//                    gir::GIRSubtype::BUFFER,
//                    {gir::BufferIR::BufferUsage::INDEX_BUFFER},
//                    getRawDataContainer().getRawDataByteArray(),
//                    getRawDataContainer().getRawDataSizeInBytes()
//            });
//        }
//
//        template<typename IndexType>
//        const std::vector<IndexType> &getIndexData() {
//            return bufferData.getRawDataAsVector<IndexType>();
//        }
//
//        template<typename IndexType>
//        void setIndexData(const std::vector<IndexType> &newIndices) {
//            bufferData.setRawDataAsArray<IndexType>(newIndices);
//        }
//
//        /**
//         *
//         * @param indexIntoBuffer this is the index into the IndexBuffer itself that you want to access
//         * @return the index stored at the location indexIntoBuffer in the index buffer
//         */
//        template<typename IndexType>
//        IndexType getIndexAt(unsigned long indexIntoBuffer) {
//            if (indexIntoBuffer >= numberOfIndices) {
//                throw std::runtime_error("Error in IndexBuffer<>::getIndexAt() - indexIntoBuffer is out of bounds!");
//            }
//            return getBufferDataAsArray<IndexType>()[indexIntoBuffer];
//        }
//
//        /**
//         *
//         * @param indexIntoBuffer
//         * @param newIndexValue
//         */
//        template<typename IndexType>
//        void setIndexAt(unsigned long indexIntoBuffer, IndexType newIndexValue) {
//            if (indexIntoBuffer >= numberOfIndices) {
//                throw std::runtime_error("Error in IndexBuffer<>::setIndexAt() - indexIntoBuffer is out of bounds!");
//            }
//            auto data = bufferData.getRawDataAsVector<IndexType>();
//            data[indexIntoBuffer] = newIndexValue;
//            bufferData.setRawDataAsArray<IndexType>(data);
//        }
//
//    private:
//        unsigned long numberOfIndices;
//
//        // TODO - figure out how updating these will work later
//    };
}
