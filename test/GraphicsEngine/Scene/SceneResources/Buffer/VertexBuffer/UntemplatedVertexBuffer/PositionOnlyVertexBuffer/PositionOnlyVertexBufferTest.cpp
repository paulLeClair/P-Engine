
#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionOnlyVertexBuffer/PositionOnlyVertexBuffer.hpp"

using namespace util;

class PositionOnlyVertexBufferTest : public ::testing::Test {
protected:
    const std::vector<VertexTypes::PositionOnlyVertex> TEST_DATA = {
            {PositionType(1.0, 1.0, 1.0)},
            {PositionType(2.0, 2.0, 2.0)},
            {PositionType(3.0, 3.0, 3.0)},
            {PositionType(4.0, 4.0, 4.0)}
    };

    void SetUp() override {
//        scene = std::make_shared<PScene>(PScene::CreationInput{
//
//        });

//        buffer = std::make_shared<PositionOnlyVertexBuffer>(PositionOnlyVertexBuffer::CreationInput{
//                scene,
//                "test",
//                UniqueIdentifier(),
//                [&](const Buffer &) {
//                    // empty update
//                },
//                TEST_DATA
//        });
    }

//    std::shared_ptr<PScene> scene;

//    std::shared_ptr<PositionOnlyVertexBuffer> buffer;
};

TEST_F(PositionOnlyVertexBufferTest, BasicCreation) {
//    ASSERT_TRUE(buffer->getName() == "test");
//
//    int index = 0;
//    for (auto &testVertex: TEST_DATA) {
//        ASSERT_EQ(testVertex, buffer->getPositionOnlyVertexDataPointer()[index]);
//        index++;
//    }
}

TEST_F(PositionOnlyVertexBufferTest, TestDifferentFormsOfDataAccess) {
    //TODO
}