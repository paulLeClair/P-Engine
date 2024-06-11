//
// Created by paull on 2023-10-27.
//

#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderGraph.hpp"
#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderPass/RenderPass.hpp"
#include "../../../../src/GraphicsEngine/Scene/ShaderModule/ShaderModule.hpp"
#include "../../../../src/GraphicsEngine/Scene/RenderGraph/RenderPass/DynamicRenderPass/DynamicRenderPass.hpp"

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
//    ASSERT_NO_THROW(
//            singleEmptyRenderPass
//                    = std::make_shared<graph::renderPass::DynamicRenderPass>(
//                    graph::renderPass::RenderPass::CreationInput{
//                            testName,
//                            uniqueIdentifier,
//                            RenderPass::Subtype::DYNAMIC_RENDER_PASS,
//                            {} // empty to use defaults
//                    })
//    );

    // add empty pass to test graph
    ASSERT_NO_THROW(
            renderGraph->addNewRenderPass(singleEmptyRenderPass)
    );

    // test that the graph has one empty pass
    ASSERT_FALSE(renderGraph->getRenderPasses().empty());
    ASSERT_TRUE(renderGraph->getRenderPasses().size() == 1);
    ASSERT_TRUE(renderGraph->getRenderPasses()[0]->getName() == testName);
    ASSERT_TRUE(renderGraph->getRenderPasses()[0]->getUniqueIdentifier() == uniqueIdentifier);

    // confirm that the bake should fail because of missing required information (eg vertex shader)
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation>> renderGraphIR;
    ASSERT_THROW(renderGraphIR = renderGraph->bakeRenderGraphToGIR(
            {}, // no resources in this test scenario
            {} // no shader modules in this test scenario
    ),
                 std::runtime_error); // TODO - update this when we improve logging/error handling etc

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
    std::shared_ptr<graph::renderPass::RenderPass> secondEmptyRenderPass;
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

    // add shaders to render passes
    ASSERT_NO_THROW(firstEmptyRenderPass->setVertexShaderModule(graph::renderPass::ShaderAttachment{
            simpleVertexShaderModule
    }));
    ASSERT_NO_THROW(firstEmptyRenderPass->setFragmentShaderModule(graph::renderPass::ShaderAttachment{
            simpleFragmentShaderModule
    }));
    ASSERT_NO_THROW(secondEmptyRenderPass->setVertexShaderModule(graph::renderPass::ShaderAttachment{
            simpleVertexShaderModule
    }));
    ASSERT_NO_THROW(secondEmptyRenderPass->setFragmentShaderModule(graph::renderPass::ShaderAttachment{
            simpleFragmentShaderModule
    }));

    // add first empty pass to test graph
    ASSERT_NO_THROW(
            renderGraph->addNewRenderPass(firstEmptyRenderPass)
    );
    ASSERT_NO_THROW(
            renderGraph->addNewRenderPass(secondEmptyRenderPass)
    );

    // test that render pass returns correct values
    ASSERT_FALSE(renderGraph->getRenderPasses().empty());
    ASSERT_TRUE(renderGraph->getRenderPasses().size() == 2);
    ASSERT_TRUE(renderGraph->getRenderPasses()[0]->getName() == testNameOne);
    ASSERT_TRUE(renderGraph->getRenderPasses()[1]->getName() == testNameTwo);
    ASSERT_TRUE(renderGraph->getRenderPasses()[0]->getUniqueIdentifier() == uniqueIdentifierOne);
    ASSERT_TRUE(renderGraph->getRenderPasses()[1]->getUniqueIdentifier() == uniqueIdentifierTwo);

    // attempt to bake the graph
    std::vector<std::shared_ptr<pEngine::girEngine::gir::GraphicsIntermediateRepresentation>> graphGirs = {};
    ASSERT_NO_THROW(graphGirs = renderGraph->bakeRenderGraphToGIR({}, {simpleFragmentShaderModule->bakeToGIR(),
                                                                       simpleVertexShaderModule->bakeToGIR()}));
}

// the render graph itself doesn't do much beyond these tasks for now; add more robust tests that use
// actually functioning
