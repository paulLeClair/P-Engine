//
// Created by paull on 2023-04-02.
//

#pragma once

#include <memory>
#include <functional>
#include "../../SceneResource.hpp"
#include "../Buffer.hpp"

/**
 * Storage buffers are slower than Uniform buffers but they can be a lot bigger. Example use case: storing all objects
 * in a scene.
 */
class StorageBuffer : public Buffer {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;

        unsigned char *initialDataPointer = nullptr;
        unsigned long initialDataSizeInBytes = 0;
    };

    explicit StorageBuffer(const CreationInput &creationInput) : Buffer(Buffer::CreationInput{
            creationInput.parentScene,
            creationInput.name,
            creationInput.uniqueIdentifier,
            creationInput.updateCallback
    }) {

    }

    bool isStorageBuffer() const override {
        return true;
    }

    [[nodiscard]] unsigned long getSizeInBytes() const override {
        return rawDataContainer->getRawDataSizeInBytes();
    }

private:
    std::shared_ptr<RawDataContainer> rawDataContainer;


};
