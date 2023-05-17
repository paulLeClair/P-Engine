#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"

using namespace PGraphics;

class ModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{});
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<Model> model;
};

TEST_F(ModelTest, BasicCreation) {
    ASSERT_NO_THROW(model = std::make_shared<Model>(Model::CreationInput{
            scene,
            "test",
            PUtilities::UniqueIdentifier(),
            {},
            {},
            {},
            {},
            {},
            {}
    }));
}