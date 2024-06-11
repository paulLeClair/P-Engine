//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/GraphicsIR/RenderGraphIR/RenderGraphIR.hpp"
#include "../../../../src/GraphicsEngine/GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp"

using namespace pEngine::girEngine::gir;
using namespace pEngine::girEngine::gir::renderPass;

class RenderGraphIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<RenderGraphIR> renderGraph;
};

TEST_F(RenderGraphIRTest, BasicCreationOfEmptyRenderGraph) {
    ASSERT_NO_THROW(renderGraph = std::make_shared<RenderGraphIR>(RenderGraphIR::CreationInput{
            "testGraph",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::RENDER_GRAPH,
            {} // no render passes
    }));
}

TEST_F(RenderGraphIRTest, BasicCreationOfNonemptyRenderGraph) {
    ASSERT_NO_THROW(renderGraph = std::make_shared<RenderGraphIR>(RenderGraphIR::CreationInput{
            "testGraph",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::RENDER_GRAPH,
            // supply one test render pass
            {
                    std::make_shared<DynamicRenderPassIR>(DynamicRenderPassIR::CreationInput{
                            "testRenderPass",
                            pEngine::util::UniqueIdentifier(),
                            GIRSubtype::RENDER_PASS,
                            RenderPassIR::RenderPassSubtype::DYNAMIC
                            // default everything else
                    })
            }
    }));
}