#include "gtest/gtest.h"

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Material/Material.hpp"

using namespace pEngine::girEngine::scene;

class MaterialTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<Material> material;
};

TEST_F(MaterialTest, BasicCreation) {
}

TEST_F(MaterialTest, CreateMaterialWithOneOfEachSubresourceType) {
    // TODO
}

// TODO - extend these tests as the overall resource creation & baking processes are fleshed out


