//
// Created by paull on 2022-06-14.
//

#include "../../../../../../src/GraphicsEngine/Scene/SceneRenderGraph/RenderPass/PRenderPass/PRenderPass.hpp"

#include <gtest/gtest.h>

TEST(PRenderPassTestSuite, CreateRenderPass) {
    PGraphics::PRenderPass::CreationInput createInfo("testPass");

    ASSERT_NO_THROW(PGraphics::PRenderPass renderPass(createInfo));
}

class PRenderPassTest : public ::testing::Test {
protected:
    PRenderPassTest() {
        PGraphics::PRenderPass::CreationInput createInfo("testPass");
        testPass = std::make_shared<PGraphics::PRenderPass>(createInfo);

        // add some test bindings
        /* SceneResource Bindings */
        testPass->addColorOutputBinding("colorOutput");
        testPass->addDepthStencilInputBinding("dsInput");
        testPass->addDepthStencilOutputBinding("dsOutput");
        testPass->addStorageImageInputBinding("storageImageInput");
        testPass->addStorageImageOutputBinding("storageImageOutput");
        testPass->addUniformBufferInputBinding("uniformBufferInput");
        testPass->addUniformTexelBufferInputBinding("uniformTexelBufferInput");
        testPass->addStorageBufferInputBinding("storageBufferInput");
        testPass->addStorageBufferOutputBinding("storageBufferOutput");
        testPass->addStorageTexelBufferInputBinding("storageTexelBuffer");
        testPass->addStorageTexelBufferOutputBinding("storageTexelBufferOutputBinding");

        /* Shader Bindings */
        testPass->setVertexShaderBinding("vert");
        testPass->setFragmentShaderBinding("frag");
        testPass->setTessellationEvaluationShaderBinding("tesE");
        testPass->setTessellationControlShaderBinding("tesC");
        testPass->setGeometryShaderBinding("geom");
    }

    ~PRenderPassTest() override = default;

    std::shared_ptr<PGraphics::PRenderPass> testPass = nullptr;
};

TEST_F(PRenderPassTest, PassCreation) {
    ASSERT_EQ(testPass->getName(), "testPass");
}

TEST(PRenderPassTestSuite, InvalidPassName) {
    PGraphics::PRenderPass::CreationInput createInfo("");
    ASSERT_THROW(PGraphics::PRenderPass invalidPass = PGraphics::PRenderPass(createInfo), std::runtime_error);
}

TEST_F(PRenderPassTest, TestValidResourceBindings) {
    const auto &resourceBindings = testPass->getResourceBindings();

    // make sure binding arrays have size of 1
    ASSERT_EQ(resourceBindings.colorOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.depthStencilInputs.size(), 1);
    ASSERT_EQ(resourceBindings.depthStencilOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageImageInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageImageOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.uniformBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.uniformTexelBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageBufferOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageTexelBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageTexelBufferOutputs.size(), 1);

    // test actual values inside binding arrays
    ASSERT_EQ(resourceBindings.colorOutputs[0], "colorOutput");
    ASSERT_EQ(resourceBindings.depthStencilInputs[0], "dsInput");
    ASSERT_EQ(resourceBindings.depthStencilOutputs[0], "dsOutput");
    ASSERT_EQ(resourceBindings.storageImageInputs[0], "storageImageInput");
    ASSERT_EQ(resourceBindings.storageImageOutputs[0], "storageImageOutput");
    ASSERT_EQ(resourceBindings.uniformBufferInputs[0], "uniformBufferInput");
    ASSERT_EQ(resourceBindings.uniformTexelBufferInputs[0], "uniformTexelBufferInput");
    ASSERT_EQ(resourceBindings.storageBufferInputs[0], "storageBufferInput");
    ASSERT_EQ(resourceBindings.storageBufferOutputs[0], "storageBufferOutput");
    ASSERT_EQ(resourceBindings.storageTexelBufferInputs[0], "storageTexelBuffer");
    ASSERT_EQ(resourceBindings.storageTexelBufferOutputs[0], "storageTexelBufferOutputBinding");
}

TEST_F(PRenderPassTest, TestValidShaderBindings) {
    const auto &shaderBindings = testPass->getShaderBindings();
    ASSERT_EQ(shaderBindings.vertexShader, "vert");
    ASSERT_EQ(shaderBindings.fragmentShader, "frag");
    ASSERT_EQ(shaderBindings.tessellationEvaluationShader, "tesE");
    ASSERT_EQ(shaderBindings.tessellationControlShader, "tesC");
    ASSERT_EQ(shaderBindings.geometryShader, "geom");
}

TEST_F(PRenderPassTest, TestInvalidResourceBindings) {
    ASSERT_EQ(testPass->addColorOutputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addDepthStencilInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addDepthStencilOutputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageImageInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageImageOutputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addUniformBufferInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addUniformTexelBufferInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageBufferInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageBufferOutputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageTexelBufferInputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->addStorageTexelBufferOutputBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);

    // make sure binding arrays have size of 1
    const auto &resourceBindings = testPass->getResourceBindings();
    ASSERT_EQ(resourceBindings.colorOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.depthStencilInputs.size(), 1);
    ASSERT_EQ(resourceBindings.depthStencilOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageImageInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageImageOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.uniformBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.uniformTexelBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageBufferOutputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageTexelBufferInputs.size(), 1);
    ASSERT_EQ(resourceBindings.storageTexelBufferOutputs.size(), 1);
}

TEST_F(PRenderPassTest, TestInvalidShaderBindings) {
    ASSERT_EQ(testPass->setVertexShaderBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->setFragmentShaderBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->setTessellationEvaluationShaderBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->setTessellationControlShaderBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
    ASSERT_EQ(testPass->setGeometryShaderBinding(""), PGraphics::PRenderPass::AddBindingResult::FAILURE);
}