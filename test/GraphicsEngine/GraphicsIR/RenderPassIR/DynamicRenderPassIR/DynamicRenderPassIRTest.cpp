//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../../src/GraphicsEngine/GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp"

using namespace pEngine::girEngine::gir;
using namespace pEngine::girEngine::gir::renderPass;

class DynamicRenderPassIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<DynamicRenderPassIR> renderPass = nullptr;
};

TEST_F(DynamicRenderPassIRTest, BasicCreation) {
    ASSERT_NO_THROW(renderPass = std::make_shared<DynamicRenderPassIR>(DynamicRenderPassIR::CreationInput{
            "testDynamicRenderPass",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::RENDER_PASS,
            RenderPassIR::RenderPassSubtype::DYNAMIC
            // defaults
    }));
}

// TODO - add more tests for any dynamic render pass specific stuff if necessary