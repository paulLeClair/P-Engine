//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../src/GraphicsEngine/GraphicsIR/GraphicsIntermediateRepresentation.hpp"

using namespace pEngine::girEngine::gir;

class GraphicsIntermediateRepresentationTest : public ::testing::Test {
protected:
    void SetUp() override {

    }

    std::shared_ptr<GraphicsIntermediateRepresentation> testGir = nullptr;
};

TEST_F(GraphicsIntermediateRepresentationTest, BasicCreation) {
    std::string testName = "test";
    const pEngine::util::UniqueIdentifier &uniqueIdentifier = pEngine::util::UniqueIdentifier();
    GIRSubtype girSubtype = GIRSubtype::UNKNOWN;
    ASSERT_NO_THROW(
            testGir = std::make_shared<GraphicsIntermediateRepresentation>(
                    GraphicsIntermediateRepresentation::CreationInput{
                            testName,
                            uniqueIdentifier,
                            girSubtype
                    })
    );
    ASSERT_EQ(testName, testGir->getName());
    ASSERT_EQ(uniqueIdentifier, testGir->getUid());
    ASSERT_EQ(girSubtype, testGir->getSubtype());
}

