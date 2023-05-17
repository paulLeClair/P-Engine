#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Scene/PScene/PScene.hpp"
#include "../../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexBuffer.hpp"

using namespace PGraphics;

class TemplatedVertexBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;
};

TEST_F(TemplatedVertexBufferTest, BasicCreationOfPositionOnlyVertexBuffer) {
    std::shared_ptr<VertexBuffer<VertexTypes::PositionOnlyVertex>> positionOnlyVertexBuffer;

    static const std::vector<VertexTypes::PositionOnlyVertex> TEST_DATA = {
            VertexTypes::PositionOnlyVertex({PositionType(1.0, 1.0, 1.0)}),
            VertexTypes::PositionOnlyVertex({PositionType(2.0, 2.0, 2.0)}),
            VertexTypes::PositionOnlyVertex({PositionType(3.0, 3.0, 3.0)})
    };

    ASSERT_NO_THROW(
            positionOnlyVertexBuffer = std::make_shared<VertexBuffer<VertexTypes::PositionOnlyVertex>>(
                    VertexBuffer<VertexTypes::PositionOnlyVertex>::CreationInput{
                            scene,
                            "testPositionOnlyVertex",
                            UniqueIdentifier(),
                            [&](const Buffer &) {
                                // nothing
                            },
                            TEST_DATA
                    }
            );
    );

    ASSERT_EQ(TEST_DATA.size(), positionOnlyVertexBuffer->getNumberOfVertices());
    ASSERT_EQ(sizeof(VertexTypes::PositionOnlyVertex), positionOnlyVertexBuffer->getVertexSizeInBytes());
    ASSERT_EQ(positionOnlyVertexBuffer->getVertexTypeToken(), VertexTypeToken::POSITION_ONLY);
    unsigned long index = 0;
    for (auto &vertex: TEST_DATA) {
        ASSERT_EQ(vertex, positionOnlyVertexBuffer->getVertex(index));
        index++;
    }
}

TEST_F(TemplatedVertexBufferTest, TestChangingVertexData) {
    static const std::vector<VertexTypes::PositionColorNormalVertex> TEST_DATA = {
            VertexTypes::PositionColorNormalVertex{
                    PositionType{1.0, 1.0, 1.0},
                    ColorType{0.1},
                    NormalType{1.0, 1.0, 1.0}
            },
            VertexTypes::PositionColorNormalVertex{
                    PositionType{2.0, 2.0, 2.0},
                    ColorType{0.2},
                    NormalType{2.0, 2.0, 2.0}
            },
            VertexTypes::PositionColorNormalVertex{
                    PositionType{3.0, 3.0, 3.0},
                    ColorType{0.3},
                    NormalType{3.0, 3.0, 3.0}
            },
            VertexTypes::PositionColorNormalVertex{
                    PositionType{4.0, 4.0, 4.0},
                    ColorType{0.4},
                    NormalType{4.0, 4.0, 4.0}
            },
            VertexTypes::PositionColorNormalVertex{
                    PositionType{5.0, 5.0, 5.0},
                    ColorType{0.5},
                    NormalType{5.0, 5.0, 5.0}
            }
    };

    std::shared_ptr<VertexBuffer<VertexTypes::PositionColorNormalVertex>> buffer;
    ASSERT_NO_THROW(
            buffer = std::make_shared<VertexBuffer<VertexTypes::PositionColorNormalVertex>>(
                    VertexBuffer<VertexTypes::PositionColorNormalVertex>::CreationInput{
                            scene,
                            "testPCNVertexBuffer",
                            UniqueIdentifier(),
                            [&](const Buffer &) {
                                //nothing
                            },
                            TEST_DATA
                    }
            )
    );

    ASSERT_EQ(TEST_DATA.size(), buffer->getNumberOfVertices());
    ASSERT_EQ(buffer->getVertexTypeToken(), VertexTypeToken::POSITION_COLOR_NORMAL);

    unsigned int index = 0;
    for (auto &vertex: TEST_DATA) {
        ASSERT_EQ(vertex, buffer->getVertexData()[index]);
        index++;
    }

    // try to set one vertex to the value of another
    ASSERT_NO_THROW(buffer->setVertex(buffer->getVertex(0), 1));
    ASSERT_EQ(buffer->getVertexData()[1], buffer->getVertex(0));

    auto modifiedTestData = TEST_DATA;

    for (auto &vertex: modifiedTestData) {
        vertex.position.x++;
        vertex.color++;
    }

    ASSERT_NO_THROW(
            buffer->setVertexData(modifiedTestData);
    );

    index = 0;
    for (auto &vertex: modifiedTestData) {
        ASSERT_EQ(vertex, buffer->getVertexData()[index]);
        ASSERT_NE(vertex, TEST_DATA[index]);
        index++;
    }
}