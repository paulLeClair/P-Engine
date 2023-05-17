#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/IndexBuffer.hpp"

class IndexBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        scene = std::make_shared<PScene>(PScene::CreationInput{

        });
    }

    std::shared_ptr<PScene> scene;

};

TEST_F(IndexBufferTest, CreateCharIndexBuffer) {
    static const std::vector<unsigned char> TEST_DATA = {
            1,
            2,
            3
    };

    auto charIndexBuffer = std::make_shared<IndexBuffer<unsigned char>>(IndexBuffer<unsigned char>::CreationInput{
            scene,
            "name",
            UniqueIdentifier(),
            [&](const Buffer &) {
                // nothing
            },
            TEST_DATA
    });

    ASSERT_TRUE(charIndexBuffer->getIndexTypeToken() == IndexTypeToken::UNSIGNED_CHAR);
    ASSERT_TRUE(charIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
    ASSERT_TRUE(charIndexBuffer->getIndexSizeInBytes() == sizeof(unsigned char));
    ASSERT_TRUE(charIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned char));
}

TEST_F(IndexBufferTest, CreateIntIndexBuffer) {
    static const std::vector<unsigned int> TEST_DATA = {
            1,
            2,
            3
    };

    auto intIndexBuffer = std::make_shared<IndexBuffer<unsigned int>>(IndexBuffer<unsigned int>::CreationInput{
            scene,
            "name",
            UniqueIdentifier(),
            [&](const Buffer &) {
                // nothing
            },
            TEST_DATA
    });

    ASSERT_TRUE(intIndexBuffer->getIndexTypeToken() == IndexTypeToken::UNSIGNED_INT);
    ASSERT_TRUE(intIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
    ASSERT_TRUE(intIndexBuffer->getIndexSizeInBytes() == sizeof(unsigned int));
    ASSERT_TRUE(intIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned int));
}

TEST_F(IndexBufferTest, CreateLongIndexBuffer) {
    static const std::vector<unsigned long> TEST_DATA = {
            1,
            2,
            3
    };

    auto longIndexBuffer = std::make_shared<IndexBuffer<unsigned long>>(IndexBuffer<unsigned long>::CreationInput{
            scene,
            "name",
            UniqueIdentifier(),
            [&](const Buffer &) {
                // nothing
            },
            TEST_DATA
    });

    ASSERT_TRUE(longIndexBuffer->getIndexTypeToken() == IndexTypeToken::UNSIGNED_LONG);
    ASSERT_TRUE(longIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
    ASSERT_TRUE(longIndexBuffer->getIndexSizeInBytes() == sizeof(unsigned long));
    ASSERT_TRUE(longIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned long));
}

TEST_F(IndexBufferTest, TestChangingIndexData) {
    static const std::vector<unsigned int> TEST_DATA = {
            1,
            2,
            3
    };

    std::shared_ptr<IndexBuffer<unsigned int>> buffer;
    ASSERT_NO_THROW(
            buffer = std::make_shared<IndexBuffer<unsigned int>>(
                    IndexBuffer<unsigned int>::CreationInput{
                            scene,
                            "testPCNIndexBuffer",
                            UniqueIdentifier(),
                            [&](const Buffer &) {
                                //nothing
                            },
                            TEST_DATA
                    }
            )
    );

    ASSERT_EQ(TEST_DATA.size(), buffer->getNumberOfIndices());
    ASSERT_EQ(buffer->getIndexTypeToken(), IndexTypeToken::UNSIGNED_INT);

    unsigned int index = 0;
    for (auto &Index: TEST_DATA) {
        ASSERT_EQ(Index, buffer->getIndexData()[index]);
        index++;
    }

    // try to set one Index to the value of another
    ASSERT_NO_THROW(buffer->setIndexAt(buffer->getIndexAt(0), 1));
    ASSERT_EQ(buffer->getIndexData()[1], buffer->getIndexAt(0));

    auto modifiedTestData = TEST_DATA;

    for (auto &testIndex: modifiedTestData) {
        testIndex++;
    }

    ASSERT_NO_THROW(
            buffer->setIndexData(modifiedTestData);
    );

    index = 0;
    for (auto &testIndex: modifiedTestData) {
        ASSERT_EQ(testIndex, buffer->getIndexData()[index]);
        ASSERT_NE(testIndex, TEST_DATA[index]);
        index++;
    }
}