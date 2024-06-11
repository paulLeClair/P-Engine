#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexBuffer.hpp"

using namespace pEngine::girEngine::scene;

class TemplatedVertexBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

};

TEST_F(TemplatedVertexBufferTest, BasicCreationOfPositionOnlyVertexBuffer) {
//    std::shared_ptr<VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex>> positionOnlyVertexBuffer;
//
//    static const std::vector<pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex> TEST_DATA = {
//            pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex({PositionType(1.0, 1.0, 1.0)}),
//            pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex({PositionType(2.0, 2.0, 2.0)}),
//            pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex({PositionType(3.0, 3.0, 3.0)})
//    };
//
//    ASSERT_NO_THROW(
//            positionOnlyVertexBuffer = std::make_shared<VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex>>(
//                    VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex>::CreationInput{
//                            "testPositionOnlyVertex",
//                            pEngine::util::UniqueIdentifier(),
//                            Buffer::BufferSubtype::VERTEX_BUFFER,
//                            TEST_DATA
//                    }
//            );
//    );
//
//    ASSERT_EQ(TEST_DATA.size(), positionOnlyVertexBuffer->getNumberOfVertices());
//    ASSERT_EQ(sizeof(pEngine::girEngine::scene::vertexTypes::PositionOnlyVertex),
//              positionOnlyVertexBuffer->getVertexSizeInBytes());
//    unsigned long index = 0;
//    for (auto &vertex: TEST_DATA) {
//        ASSERT_EQ(vertex, positionOnlyVertexBuffer->getVertex(index));
//        index++;
//    }
}

TEST_F(TemplatedVertexBufferTest, TestChangingVertexData) {
//    static const std::vector<pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex> TEST_DATA = {
//            pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex{
//                    PositionType{1.0, 1.0, 1.0},
//                    ColorType{0.1},
//                    NormalType{1.0, 1.0, 1.0}
//            },
//            pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex{
//                    PositionType{2.0, 2.0, 2.0},
//                    ColorType{0.2},
//                    NormalType{2.0, 2.0, 2.0}
//            },
//            pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex{
//                    PositionType{3.0, 3.0, 3.0},
//                    ColorType{0.3},
//                    NormalType{3.0, 3.0, 3.0}
//            },
//            pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex{
//                    PositionType{4.0, 4.0, 4.0},
//                    ColorType{0.4},
//                    NormalType{4.0, 4.0, 4.0}
//            },
//            pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex{
//                    PositionType{5.0, 5.0, 5.0},
//                    ColorType{0.5},
//                    NormalType{5.0, 5.0, 5.0}
//            }
//    };
//
//    std::shared_ptr<VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex>> buffer;
//    ASSERT_NO_THROW(
//            buffer = std::make_shared<VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex>>(
//                    VertexBuffer<pEngine::girEngine::scene::vertexTypes::PositionColorNormalVertex>::CreationInput{
//                            "testPCNVertexBuffer",
//                            pEngine::util::UniqueIdentifier(),
//                            Buffer::BufferSubtype::VERTEX_BUFFER,
//                            TEST_DATA
//                    }
//            )
//    );
//
//    ASSERT_EQ(TEST_DATA.size(), buffer->getNumberOfVertices());
//
//    unsigned int index = 0;
//    for (auto &vertex: TEST_DATA) {
//        ASSERT_EQ(vertex, buffer->getVertexData()[index]);
//        index++;
//    }
//
//    // try to set one vertex to the value of another
//    ASSERT_NO_THROW(buffer->setVertexAt(1, buffer->getVertex(0)));
//    ASSERT_EQ(buffer->getVertexData()[1], buffer->getVertex(0));
//
//    auto modifiedTestData = TEST_DATA;
//
//    for (auto &vertex: modifiedTestData) {
//        vertex.position.x++;
//        vertex.color++;
//    }
//
//    ASSERT_NO_THROW(
//            buffer->setVertexData(modifiedTestData);
//    );
//
//    index = 0;
//    for (auto &vertex: modifiedTestData) {
//        ASSERT_EQ(vertex, buffer->getVertexData()[index]);
//        ASSERT_NE(vertex, TEST_DATA[index]);
//        index++;
//    }
}