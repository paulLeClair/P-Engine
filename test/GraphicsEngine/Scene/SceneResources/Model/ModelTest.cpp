#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Model/Model.hpp"

// DEBUGGING
#ifdef _WIN32

#include <Windows.h>

#endif

using namespace pEngine::girEngine::scene;
using namespace pEngine::util;

class ModelTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<model::Model> model;
};

TEST_F(ModelTest, BasicCreationOfAssimpModel) {
    // OKAY - for the bonegroup rewrite and assimp model loading in general, we want this test
    // to basically attempt to load in an assimp model; user/externally-specified models will come later
}
