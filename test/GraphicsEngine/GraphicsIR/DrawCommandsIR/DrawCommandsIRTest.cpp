//
// Created by paull on 2023-10-31.
//

#include <gtest/gtest.h>
#include "../../../../src/GraphicsEngine/GraphicsIR/DrawAttachmentIR/DrawAttachmentIR.hpp"
#include "../../../../src/lib/glm/vec4.hpp"

using namespace pEngine::girEngine::gir;

class DrawCommandsIRTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<DrawAttachmentIR> drawCommands;
};

TEST_F(DrawCommandsIRTest, BasicCreationOfEmptyDrawCommandsGir) {
    std::string testName = "testEmptyDrawCommandsIR";
    pEngine::util::UniqueIdentifier uniqueIdentifier = pEngine::util::UniqueIdentifier();
    DrawAttachmentIR::DrawType drawType = DrawAttachmentIR::DrawType::UNKNOWN;
    ASSERT_NO_THROW(drawCommands = std::make_shared<DrawAttachmentIR>(DrawAttachmentIR::CreationInput{
            testName,
            uniqueIdentifier,
            GIRSubtype::DRAW_COMMANDS,
            drawType,
            {}, // no vertices
            {} // no indices
    }));

    ASSERT_EQ(testName, drawCommands->getName());
    ASSERT_EQ(uniqueIdentifier, drawCommands->getUid());
    ASSERT_EQ(drawType, drawCommands->getDrawType());
    ASSERT_EQ(GIRSubtype::DRAW_COMMANDS, drawCommands->getSubtype());
}

TEST_F(DrawCommandsIRTest, BasicCreationOfNonemptyGir) {
    static const std::vector<glm::vec4> vertices = {
            // four points of a square
            glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
            glm::vec4(0.5f, -0.5f, 0.0f, 1.0f),
            glm::vec4(0.5f, 0.5f, 0.0f, 1.0f),
            glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f)
    };

    static const std::vector<unsigned int> indices = {
            // triangle 0
            0,
            1,
            2,
            // triangle 1
            2,
            3,
            0
    };

    static const std::vector<std::shared_ptr<BufferIR> > &testNonemptyGirVertexBuffers = {
            std::make_shared<BufferIR>(BufferIR::CreationInput{
                    "testVertices",
                    pEngine::util::UniqueIdentifier(),
                    GIRSubtype::BUFFER,
                    {BufferIR::BufferUsage::VERTEX_BUFFER},
                    reinterpret_cast<const unsigned char *>(vertices.data()),
                    static_cast<unsigned long>(vertices.size())
            })
    };

    static const std::vector<std::shared_ptr<BufferIR> > &testNonemptyGirIndexBuffers = {
            std::make_shared<BufferIR>(BufferIR::CreationInput{
                    "testIndices",
                    pEngine::util::UniqueIdentifier(),
                    GIRSubtype::BUFFER,
                    {BufferIR::BufferUsage::INDEX_BUFFER},
                    reinterpret_cast<const unsigned char *>(indices.data()),
                    static_cast<unsigned long>(indices.size())
            })
    };

    static const std::string testName = "testNonemptyDrawCommandsIR";
    static const pEngine::util::UniqueIdentifier uniqueIdentifier = pEngine::util::UniqueIdentifier();
    static const DrawAttachmentIR::DrawType drawType = DrawAttachmentIR::DrawType::INDEXED_DRAW;

//    ASSERT_NO_THROW(drawCommands = std::make_shared<DrawAttachmentIR>(DrawAttachmentIR::CreationInput{
//            testName,
//            uniqueIdentifier,
//            GIRSubtype::DRAW_COMMANDS,
//            drawType,
//            testNonemptyGirVertexBuffers,
//            testNonemptyGirIndexBuffers
//    }));
//
//    ASSERT_EQ(testName, drawCommands->getName());
//    ASSERT_EQ(uniqueIdentifier, drawCommands->getUid());
//    ASSERT_EQ(drawType, drawCommands->getDrawType());
//    ASSERT_EQ(GIRSubtype::DRAW_COMMANDS, drawCommands->getSubtype());
//    ASSERT_EQ(testNonemptyGirVertexBuffers, drawCommands->getVertexBuffers());
//    ASSERT_EQ(testNonemptyGirIndexBuffers, drawCommands->getIndexBuffers());
}
