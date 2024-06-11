#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Model/Model.hpp"

using namespace pEngine::girEngine::scene;

class ModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<Model> model;
};

TEST_F(ModelTest, BasicCreation) {
    ASSERT_NO_THROW(model = std::make_shared<Model>(Model::CreationInput{
            "test",
            pEngine::util::UniqueIdentifier(),
            {},
            {},
            {},
            {},
            {},
            {}
    }));
}