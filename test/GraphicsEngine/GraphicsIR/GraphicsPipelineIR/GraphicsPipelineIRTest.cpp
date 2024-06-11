//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/GraphicsPipelineIR.hpp"

using namespace pEngine::girEngine::gir;

class GraphicsPipelineIRTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<GraphicsPipelineIR> graphicsPipeline = nullptr;
};

TEST_F(GraphicsPipelineIRTest, BasicCreation) {
    ASSERT_NO_THROW(graphicsPipeline = std::make_shared<GraphicsPipelineIR>(GraphicsPipelineIR::CreationInput{
            "testPipeline",
            pEngine::util::UniqueIdentifier(),
            GIRSubtype::GRAPHICS_PIPELINE
            // defaults
    }));
}

// TODO - add more tests that mess around with particular settings; GIR classes generally just store info anyway so there won't be much logic