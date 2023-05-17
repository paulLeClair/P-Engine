//
// Created by paull on 2023-01-08.
//

#pragma once


#include <functional>
#include "../SceneView.hpp"

#include "../../../../lib/glm/glm.hpp"
#include "../../../../lib/glm/ext/quaternion_double.hpp"
#include "../../../../lib/glm/vec3.hpp"

class PSceneView : public SceneView {
public:
    struct CreationInput {
        std::shared_ptr<Scene> parentScene;
        std::string name;
        PUtilities::UniqueIdentifier uniqueIdentifier;
        glm::vec3 initialPosition;
        glm::dquat initialRotationQuaternion;

        std::function<void(const PSceneView &)> updateCallback;
    };

    explicit PSceneView(const CreationInput &creationInput) : parentScene(creationInput.parentScene),
                                                              name(creationInput.name),
                                                              uniqueIdentifier(creationInput.uniqueIdentifier),
                                                              position(creationInput.initialPosition),
                                                              rotationQuaternion(
                                                                      creationInput.initialRotationQuaternion) {

    }

    [[nodiscard]] const std::shared_ptr<Scene> &getParentScene() const override {
        return parentScene;
    }

    [[nodiscard]] const std::string &getName() const override {
        return name;
    }

    [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const override {
        return uniqueIdentifier;
    }

    [[nodiscard]] const glm::vec3 &getPosition() const {
        return position;
    }

    [[nodiscard]] const glm::dquat &getRotationQuaternion() const {
        return rotationQuaternion;
    }

    void setPosition(const glm::vec3 &newPosition) {
        position = newPosition;
    }

    void setRotationQuaternion(const glm::dquat &newRotation) {
        rotationQuaternion = newRotation;
    }

    enum class UpdateResult {
        SUCCESS,
        FAILURE
    };

    UpdateResult update() {
        try {
            updateCallback(*this);
            return UpdateResult::SUCCESS;
        }
        catch (std::exception &exception) {
            return UpdateResult::FAILURE;
        }
    }

private:
    std::shared_ptr<Scene> parentScene;

    std::string name;

    PUtilities::UniqueIdentifier uniqueIdentifier;

    glm::vec3 position; // TODO - evaluate what to do for this position representation

    glm::dquat rotationQuaternion; // TODO - evaluate whether this is fine to do (should be I think)

    std::function<void(const PSceneView &)> updateCallback;
};
