#include "gtest/gtest.h"

#include "../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"

using namespace PGraphics;

class MaterialTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

    std::shared_ptr<Material> material;
};

TEST_F(MaterialTest, BasicCreation) {
    ASSERT_NO_THROW(material = std::make_shared<Material>(Material::CreationInput{
            scene,
            "test",
            PUtilities::UniqueIdentifier(),
            {},
            {},
            {},
            {},
            {}
    }));
}

TEST_F(MaterialTest, CreateMaterialWithOneOfEachSubresourceType) {
    // TODO
}

// TODO - extend these tests as the overall resource creation & baking processes are fleshed out


