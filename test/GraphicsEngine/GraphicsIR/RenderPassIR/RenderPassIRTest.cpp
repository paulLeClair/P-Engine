//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/GraphicsIR/RenderPassIR/RenderPassIR.hpp"

using namespace pEngine::girEngine::gir;
using namespace pEngine::girEngine::gir::renderPass;

class RenderPassIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<RenderPassIR> renderPass = nullptr;
};

TEST_F(RenderPassIRTest, BasicCreationDynamicSubtype) {
    ASSERT_NO_THROW(
            renderPass = std::make_shared<RenderPassIR>(RenderPassIR::CreationInput{
                    "testRenderPass",
                    pEngine::util::UniqueIdentifier(),
                    GIRSubtype::RENDER_PASS
                    // defaults
            })
    );
}

TEST_F(RenderPassIRTest, BasicCreationStaticSubtype) {
    ASSERT_NO_THROW(
            renderPass = std::make_shared<RenderPassIR>(RenderPassIR::CreationInput{
                    "testRenderPass",
                    pEngine::util::UniqueIdentifier(),
                    GIRSubtype::RENDER_PASS
                    // defaults
            })
    );
}