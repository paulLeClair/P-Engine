//
// Created by paull on 2023-10-27.
//

#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderGraph.hpp"
#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderPass/RenderPass.hpp"
#include "../../../../src/GraphicsEngine/Scene/ShaderModule/ShaderModule.hpp"
#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderPass/DynamicRenderPass.hpp"

#include <gtest/gtest.h>

using namespace pEngine::girEngine::scene;

static const char *const TEST_VERTEX_SHADER_SPV_FILENAME = "testVertShader";

static const char *const TEST_FRAGMENT_SHADER_SPV_FILENAME = "testFragShader";

class RenderGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        renderGraph = std::make_shared<graph::RenderGraph>(graph::RenderGraph::CreationInput{
                "testRenderGraph",
                pEngine::util::UniqueIdentifier()
        });
    }

    std::shared_ptr<graph::RenderGraph> renderGraph = nullptr;
};

TEST_F(RenderGraphTest, TestBasicCreationAndIdentifiers) {
    pEngine::util::UniqueIdentifier otherUid = pEngine::util::UniqueIdentifier();
    std::shared_ptr<graph::RenderGraph> otherRenderGraph;
    ASSERT_NO_THROW(
            otherRenderGraph = std::make_shared<graph::RenderGraph>(
                    graph::RenderGraph::CreationInput{
                            "otherRenderGraph",
                            otherUid
                    })
    );

    // test that names and uids have valid values
    ASSERT_NE(renderGraph->getName(), otherRenderGraph->getName());
    ASSERT_EQ(otherRenderGraph->getUid(), otherUid);
    ASSERT_NE(renderGraph->getUid(), otherRenderGraph->getUid());
}

TEST_F(RenderGraphTest, TestSceneViews) {
    // TODO until backend bake informs what we need for scene view stuff
}

TEST_F(RenderGraphTest, AddSingleEmptyRenderPassToGraphAndBakeIt) {

    // create single empty render pass for testing
    std::shared_ptr<graph::renderPass::DynamicRenderPass> singleEmptyRenderPass;
    const char *testName = "emptyRenderPass";
    const UniqueIdentifier &uniqueIdentifier = pEngine::util::UniqueIdentifier();

    // confirm that the bake should fail because of missing required information (eg vertex shader)
//    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation>> renderGraphIR;
//    ASSERT_THROW(renderGraphIR = renderGraph->bakeRenderGraphToGIR(
//            {}, // no resources in this test scenario
//            {} // no shader modules in this test scenario
//    ),
//                 std::runtime_error); // TODO - update this when we improve logging/error handling etc
//
}

TEST_F(RenderGraphTest, CreateTwoEmptyRenderPassesAndBakeTestGraph) {

    // create first empty render pass for testing
    std::shared_ptr<graph::renderPass::DynamicRenderPass> firstEmptyRenderPass;
    const char *testNameOne = "emptyRenderPass1";
    const UniqueIdentifier &uniqueIdentifierOne = pEngine::util::UniqueIdentifier();
//    ASSERT_NO_THROW(
//            firstEmptyRenderPass
//                    = std::make_shared<graph::renderPass::DynamicRenderPass>(
//                    graph::renderPass::DynamicRenderPass::CreationInput{
//                            testNameOne,
//                            uniqueIdentifierOne,
//                            RenderPass::Subtype::DYNAMIC_RENDER_PASS,
//                            {} // empty to use defaults
//                    })
//    );

    // create second empty render pass for testing
//    std::shared_ptr<graph::renderPass::RenderPass> secondEmptyRenderPass;
    const char *testNameTwo = "emptyRenderPass2";
    const UniqueIdentifier &uniqueIdentifierTwo = pEngine::util::UniqueIdentifier();
//    ASSERT_NO_THROW(
//            secondEmptyRenderPass
//                    = std::make_shared<graph::renderPass::RenderPass>(
//                    graph::renderPass::RenderPass::CreationInput{
//                            testNameTwo,
//                            uniqueIdentifierTwo,
//                            RenderPass::Subtype::DYNAMIC_RENDER_PASS,
//                            {} // empty to use defaults
//                    })
//    );

    // create simple vertex shader
    std::shared_ptr<ShaderModule> simpleVertexShaderModule = std::make_shared<ShaderModule>(
            ShaderModule::CreationInput{
                    "emptyTestVertexShader",
                    pEngine::util::UniqueIdentifier(),
                    TEST_VERTEX_SHADER_SPV_FILENAME,
                    "main",
                    ShaderModule::ShaderUsage::VERTEX_SHADER,
                    ShaderModule::ShaderLanguage::GLSL
            });

    // create simple fragment shader
    std::shared_ptr<ShaderModule> simpleFragmentShaderModule = std::make_shared<ShaderModule>(
            ShaderModule::CreationInput{
                    "emptyTestFragmentShader",
                    pEngine::util::UniqueIdentifier(),
                    TEST_FRAGMENT_SHADER_SPV_FILENAME,
                    "main",
                    ShaderModule::ShaderUsage::FRAGMENT_SHADER,
                    ShaderModule::ShaderLanguage::GLSL
            });
}

// the render graph itself doesn't do much beyond these tasks for now; add more robust tests that use
// actually functioning
