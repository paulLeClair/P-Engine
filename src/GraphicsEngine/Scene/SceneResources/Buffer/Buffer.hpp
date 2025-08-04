//
// Created by paull on 2022-09-21.
//

#pragma once

#include <functional>
#include <any>
#include <stdexcept>

#include "../SceneResource.hpp"

#include "../../../../utilities/ByteArray/ByteArray.hpp"

#include "../../../GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp"

namespace pEngine::girEngine::scene {
    // TODO - remove the OOPness of this design and other resources (it serves no purpose and is ugly)
    // (I'll try ripping it out now actually)
    class Buffer {
    public:
        ~Buffer() = default;


        enum class BufferSubtype {
            UNKNOWN,
            INDEX_BUFFER,
            VERTEX_BUFFER,
            UNIFORM_BUFFER,
            STORAGE_BUFFER,
            TEXEL_BUFFER,
            STORAGE_TEXEL_BUFFER
        };

        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;

            BufferSubtype bufferType;

            /**
             * This should either be the vertex input binding index or the binding slot within the
             * descriptor set (note: for descriptor-bound resources you have to specify the descriptor set index below)
             */
            uint32_t bindingIndex = 0;

            unsigned char *initialDataPointer = nullptr;
            unsigned long initialDataSizeInBytes = 0;

            /**
             * This should be used for the case where you want to provide the system
             * a particular (static) upper bound on the uniform buffer size.
             *
             * Not sure if it'll end up mattering, but for the animated model demo
             * we may benefit from having this information for the animation bones uniform
             */
            boost::optional<uint32_t> optionalMaxBufferSize = boost::none;

            boost::optional<uint32_t> descriptorSetIndex = boost::none;

        };

        explicit Buffer(const CreationInput &createInfo)
            : name(createInfo.name),
              uid(createInfo.uid),
              maxBufferSize(createInfo.optionalMaxBufferSize),
              bufferSubtype(createInfo.bufferType),
              bindingIndex(createInfo.bindingIndex) {
            // 90% SURE THAT THIS IS NOT WORKING PROPERLY -> I don't think it's copying the right amount of bytes
            bufferData = util::ByteArray(util::ByteArray::CreationInput{
                createInfo.name,
                createInfo.uid,
                createInfo.initialDataPointer,
                createInfo.initialDataSizeInBytes
            });
        }


        [[nodiscard]] BufferSubtype getBufferSubtype() const {
            return bufferSubtype;
        }

        [[nodiscard]] unsigned long getSizeInBytes() const {
            return bufferData.getRawDataSizeInBytes();
        }

        [[nodiscard]] unsigned long getNumberOfDataElements() const {
            return bufferData.getNumberOfDataElements();
        }

        [[nodiscard]] uint32_t getBindingIndex() const {
            return bindingIndex;
        }

        template<typename RawDataType>
        const RawDataType *getBufferDataAsPointer() const {
            return bufferData.getRawDataPointer<const RawDataType>();
        }

        template<typename RawDataType>
        const std::vector<RawDataType> &getBufferDataAsArray() const {
            return bufferData.getRawDataAsVector<RawDataType>();
        }

        template<typename RawDataType>
        void setBufferData(RawDataType &newData) {
            setBufferData(&newData);
        }

        template<typename RawDataType>
        void setBufferData(RawDataType *newData) {
            bufferData.setRawData(newData, sizeof(RawDataType));
        }

        template<typename RawDataType>
        void setBufferDataAsArray(const std::vector<RawDataType> &newData) {
            bufferData.setRawData((unsigned char *) newData.data(), newData.size() * sizeof(RawDataType));
        }

        template<typename RawDataType>
        void setBufferDataFromPointer(const RawDataType *rawData, unsigned long long numberOfBytesToCopy) {
            bufferData.setRawData((unsigned char *) rawData, numberOfBytesToCopy);
        }

        [[nodiscard]] const util::ByteArray &getRawDataContainer() const {
            return bufferData;
        }

        [[nodiscard]] gir::BufferIR bakeBuffer() const {
            // TODO -> validation etc
            return gir::BufferIR{
                name,
                uid,
                gir::GIRSubtype::BUFFER,
                convertBufferSubtypeToGirAnalogue(bufferSubtype),
                bindingIndex,
                getRawDataContainer().getRawDataByteArray(),
                static_cast<uint32_t>(getRawDataContainer().getRawDataSizeInBytes()),
                maxBufferSize
            };
        }

        std::string name;
        util::UniqueIdentifier uid;

        boost::optional<uint32_t> maxBufferSize = boost::none;

    private: // I'll leave these private for now but it's probably not necessary

        BufferSubtype bufferSubtype;

        util::ByteArray bufferData;

        uint32_t bindingIndex;


        static constexpr gir::BufferIR::BufferUsage convertBufferSubtypeToGirAnalogue(const BufferSubtype &subtype) {
            switch (subtype) {
                case (BufferSubtype::UNIFORM_BUFFER): {
                    return gir::BufferIR::BufferUsage::UNIFORM_BUFFER;
                };
                // TODO -> support other buffer types; for the single-animated-model demo we'll trim it down
                // to just the uniform buffers that we need (for now)
                default:
                    return gir::BufferIR::BufferUsage::UNKNOWN;
            };
        }
    };
}
