#include <utility>
#include <stdexcept>

#pragma once

#include "../SceneResource.hpp"

class Renderable : public SceneResource {
public:
    enum class RenderableType {
        // since each type of renderable will need to be baked differently, we need some kind of way to denote which type each renderable is
        PointSet,
        TriangleMesh,
        IndexedTriangleMesh,
        TessellationPatch
    };

    struct CreationInput {
        std::shared_ptr<Scene> parentScene;

        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;

        RenderableType type;
    };

    explicit Renderable(const CreationInput &creationInput) : parentScene(creationInput.parentScene),
                                                              name(creationInput.name),
                                                              uniqueIdentifier(creationInput.uniqueIdentifier),
                                                              type(creationInput.type) {

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

    [[nodiscard]] RenderableType getType() const {
        return type;
    }


protected:
    std::shared_ptr<Scene> parentScene;

    std::string name;
    PUtilities::UniqueIdentifier uniqueIdentifier;

    RenderableType type;

};
