
//
//#include <gtest/gtest.h>
//
//
//using namespace pEngine::girEngine::scene;
//
//class IndexBufferTest : public ::testing::Test {
//protected:
//    void SetUp() override {
//    }
//
//
//};
//
//TEST_F(IndexBufferTest, CreateCharIndexBuffer) {
//    static const std::vector<unsigned char> TEST_DATA = {
//            1,
//            2,
//            3
////    };
//
////    auto charIndexBuffer = std::make_shared<IndexBuffer>(IndexBuffer::CreationInput{
//            "name",
//            pEngine::util::UniqueIdentifier(),
//            Buffer::BufferSubtype::INDEX_BUFFER,
//            TEST_DATA
//    });
//
//    ASSERT_TRUE(charIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
//    ASSERT_TRUE(charIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned char));
//}
//
//TEST_F(IndexBufferTest, CreateIntIndexBuffer) {
//    static const std::vector<unsigned char> TEST_DATA = {
//            1,
//            2,
//            3
//    };
//
//    auto intIndexBuffer = std::make_shared<IndexBuffer>(IndexBuffer::CreationInput{
//            "name",
//            pEngine::util::UniqueIdentifier(),
//            Buffer::BufferSubtype::INDEX_BUFFER,
//            TEST_DATA
//    });
//
//    ASSERT_TRUE(intIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
//    ASSERT_TRUE(intIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned int));
//}
//
//TEST_F(IndexBufferTest, CreateLongIndexBuffer) {
////    static const std::vector<unsigned long> TEST_DATA = {
////            1,
////            2,
////            3
////    };
////
////    auto longIndexBuffer = std::make_shared<IndexBuffer>(IndexBuffer::CreationInput{
////            "name",
////            pEngine::util::UniqueIdentifier(),
////            Buffer::BufferSubtype::INDEX_BUFFER,
////            TEST_DATA
////    });
////
////    ASSERT_TRUE(longIndexBuffer->getNumberOfIndices() == TEST_DATA.size());
////    ASSERT_TRUE(longIndexBuffer->getIndexSizeInBytes() == sizeof(unsigned long));
////    ASSERT_TRUE(longIndexBuffer->getSizeInBytes() == TEST_DATA.size() * sizeof(unsigned long));
//}
//
//TEST_F(IndexBufferTest, TestChangingIndexData) {
//    static const std::vector<unsigned int> TEST_DATA = {
//            1,
//            2,
//            3
//    };
//
//    std::shared_ptr<IndexBuffer> buffer;
//    ASSERT_NO_THROW(
//            buffer = std::make_shared<IndexBuffer>(
//                    IndexBuffer::CreationInput{
//                            "testPCNIndexBuffer",
//                            pEngine::util::UniqueIdentifier(),
//                            Buffer::BufferSubtype::INDEX_BUFFER,
//                            TEST_DATA
//                    }
//            )
//    );
//
//    ASSERT_EQ(TEST_DATA.size(), buffer->getNumberOfIndices());
//
//    unsigned int index = 0;
//    for (auto &testDatum: TEST_DATA) {
//        ASSERT_EQ(testDatum, buffer->getIndexData()[index]);
//        index++;
//    }
//
//    // try to set one Index to the value of another
//    ASSERT_NO_THROW(buffer->setIndexAt(1, buffer->getIndexAt(0)));
//    ASSERT_EQ(buffer->getIndexAt(1), buffer->getIndexAt(0));
//
//    auto modifiedTestData = TEST_DATA;
//
//    for (auto &testIndex: modifiedTestData) {
//        testIndex++;
//    }
//
//    ASSERT_NO_THROW(
//            buffer->setIndexData(modifiedTestData);
//    );
//
//    index = 0;
//    for (auto &testIndex: modifiedTestData) {
//        ASSERT_EQ(testIndex, buffer->getIndexData()[index]);
//        ASSERT_NE(testIndex, TEST_DATA[index]);
//        index++;
//    }
//}