//
// Created by paull on 2022-09-21.
//

#pragma once

#include <functional>
#include <any>

#include "../SceneResource.hpp"
#include "../../Scene.hpp"
#include "../../../../EngineCore/utilities/RawDataContainer/RawDataContainer.hpp"

/**
 * NEW DESIGN AS OF APRIL 02 2023 -
 * I think I've been trying to avoid polymorphism when I should be using it.
 *
 * Instead of the previous "BufferUsages" to differentiate and trying to have a single buffer class for all usages,
 * I'll adapt this Buffer class to be a base (abstract) class and have the different buffer types handle their data
 * more directly.
 *
 * This way when we actually bake the buffers, we'll have a better idea of the actual data that the buffer contains,
 * and we can have it be handled directly when we're actually copying the data around. I'm still kind of unsure
 * about how best to handle raw data, but I'm thinking making a "PUtilities::RawDataContainer" class
 */
class Buffer : public SceneResource {
public:
    ~Buffer() override = default;

    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        std::function<void(const Buffer &)> updateCallback;
    };

    explicit Buffer(const CreationInput &createInfo) : parentScene(createInfo.parentScene), name(createInfo.name),
                                                       uniqueIdentifier(createInfo.uniqueIdentifier),
                                                       updateCallback(createInfo.updateCallback) {

    }

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }


    UpdateResult update() override {
        try {
            updateCallback(*this);

            return UpdateResult::SUCCESS;
        }
        catch (std::exception &exception) {
            // TODO - log!
            return UpdateResult::FAILURE;
        }
    }

    [[nodiscard]] const std::function<void(const Buffer &)> &getUpdateCallback() const {
        return updateCallback;
    }

    [[nodiscard]] virtual bool isGeometryBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isVertexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isIndexBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isUniformBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isStorageBuffer() const {
        return false;
    }

    [[nodiscard]] virtual bool isTexelBuffer() const {
        return false;
    }

    [[nodiscard]] virtual unsigned long getSizeInBytes() const = 0;

protected:
    std::shared_ptr<Scene> parentScene;

    std::string name;
    PUtilities::UniqueIdentifier uniqueIdentifier;

    /**
     * NOTE - I should probably replace the argument here with a simple "void *this" pointer, which will be
     * set to be a pointer to the buffer that is calling itself
     *
     * This way you can just dynamic_pointer_cast the void* to be the Buffer subtype that you want to use and
     * update accordingly
     */
    std::function<void(const Buffer &)> updateCallback;

};
