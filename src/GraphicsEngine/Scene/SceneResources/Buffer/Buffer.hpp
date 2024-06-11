//
// Created by paull on 2022-09-21.
//

#pragma once

#include <functional>
#include <any>
#include <stdexcept>

#include "../SceneResource.hpp"

#include "../../../../utilities/RawDataContainer/RawDataContainer.hpp"

#include "../../../GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp"

namespace pEngine::girEngine::scene {

    class Buffer : public scene::Resource {
    public:
        ~Buffer() override = default;

        enum class BufferSubtype {
            UNKNOWN,
            INDEX_BUFFER,
            VERTEX_BUFFER,
            UNIFORM_BUFFER,
            STORAGE_BUFFER,
            TEXEL_BUFFER,
            STORAGE_TEXEL_BUFFER
        };

        struct CreationInput : public Resource::CreationInput {
            BufferSubtype bufferType;

            // TODO - rehash update stuff later

            unsigned char *initialDataPointer = nullptr;
            unsigned long initialDataSizeInBytes = 0;
        };

        explicit Buffer(const CreationInput &createInfo) : Resource(createInfo),
                                                           bufferSubtype(createInfo.bufferType) {
            bufferData = std::make_unique<util::RawDataContainer>(util::RawDataContainer::CreationInput{
                    createInfo.name,
                    createInfo.uid,
                    createInfo.initialDataPointer,
                    createInfo.initialDataSizeInBytes
            });
        }

        /**
         * TODO - rehash this once we get static scenes rendering
         * @return
         */
        UpdateResult update() override {
            return UpdateResult::FAILURE;
        }

        [[nodiscard]] BufferSubtype getBufferSubtype() const {
            return bufferSubtype;
        }

        [[nodiscard]] unsigned long getSizeInBytes() const {
            return bufferData->getRawDataSizeInBytes();
        }

        [[nodiscard]] unsigned long getNumberOfDataElements() const {
            return bufferData->getNumberOfDataElements();
        }

        template<typename RawDataType>
        const RawDataType *getBufferDataAsPointer() const {
            return bufferData->getRawDataPointer<const RawDataType>();
        }

        template<typename RawDataType>
        const std::vector<RawDataType> &getBufferDataAsArray() const {
            return bufferData->getRawDataAsVector<RawDataType>();
        }

        template<typename RawDataType>
        void setBufferData(RawDataType &newData) {
            setBufferData(&newData);
        }

        template<typename RawDataType>
        void setBufferData(RawDataType *newData) {
            bufferData->setRawData(newData, sizeof(RawDataType));
        }

        template<typename RawDataType>
        void setBufferDataAsArray(const std::vector<RawDataType> &newData) {
            bufferData->setRawData((unsigned char *) newData.data(), newData.size() * sizeof(RawDataType));
        }

        template<typename RawDataType>
        void setBufferDataFromPointer(const RawDataType *rawData, unsigned long long numberOfBytesToCopy) {
            bufferData->setRawData((unsigned char *) rawData, numberOfBytesToCopy);
        }

        [[nodiscard]] const util::RawDataContainer &getRawDataContainer() const {
            return *bufferData;
        }

        std::shared_ptr<gir::GraphicsIntermediateRepresentation> bakeToGIR() override {
            throw std::runtime_error("Error: Attempting to bake generic scene::Buffer to GIR!");
        }


    private:
        BufferSubtype bufferSubtype;

    protected:
        /**
         * This is protected to allow VertexBuffer<> and IndexBuffer<> to set up the data without having
         * to use the redundant Buffer::CreationInput initialization data parameters, if there's another
         * way to do it without making this private then that might be worth looking into in the future
         */
        std::unique_ptr<util::RawDataContainer> bufferData;

        /**
         * TODO - redo updates after static scenes are rendering
         */

    };

}