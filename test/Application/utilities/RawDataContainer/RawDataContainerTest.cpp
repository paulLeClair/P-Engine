//
// Created by paull on 2023-04-02.
//

#include <gtest/gtest.h>

#include "../../../../src/utilities/ByteArray/ByteArray.hpp"
#include <glm/detail/type_mat4x4.hpp>

using namespace pEngine::util;

class RawDataContainerTest : public ::testing::Test {
protected:
    struct SimpleTestData {
        glm::mat4 test4x4Matrix = glm::mat4(0);
        glm::vec3 testVec3 = glm::vec3(0);
        double testDouble = 0.5;

        bool operator==(const SimpleTestData &other) const {
            return (test4x4Matrix == other.test4x4Matrix)
                   && (testVec3 == other.testVec3)
                   && (testDouble == other.testDouble);
        }
    };

    SimpleTestData simpleTestData = {
            glm::mat4(1.0f),
            glm::vec3(1.0, 2.0, 3.0),
            4.0
    };

    void SetUp() override {

    }


};

TEST_F(RawDataContainerTest, testInvalidCreationInput) {
    ByteArray::CreationInput invalidCreationInput = {
            "",
            pEngine::util::UniqueIdentifier(),
            nullptr,
            0
    };

    ASSERT_ANY_THROW(auto rawDataContainer = std::make_shared<ByteArray>(invalidCreationInput));
}


TEST_F(RawDataContainerTest, createEmptyContainer) {
    ByteArray::CreationInput creationInput = {
            "TestRawDataContainer1",
            pEngine::util::UniqueIdentifier(),
            nullptr,
            0
    };

    std::shared_ptr<ByteArray> testContainer;
    ASSERT_NO_THROW(testContainer = std::make_shared<ByteArray>(creationInput));
}

TEST_F(RawDataContainerTest, testContainerWithSimpleDoubleData) {
    std::vector<double> someDoubles = {
            1.0,
            2.0,
            3.0,
            4.0,
            5.0,
            6.0,
            7.0,
            8.0
    };

    ByteArray::CreationInput simpleStringTestCreationInput = {
            "simple string test data",
            pEngine::util::UniqueIdentifier(),
            reinterpret_cast<unsigned char *>(&someDoubles),
            static_cast<unsigned long>(someDoubles.size() * sizeof(double))
    };

    std::shared_ptr<ByteArray> simpleStringTestContainer;
    ASSERT_NO_THROW(simpleStringTestContainer = std::make_shared<ByteArray>(simpleStringTestCreationInput));

    auto *testContainerDataPointer = simpleStringTestContainer->getRawDataPointer<std::vector<double>>();
    std::vector<double> testContainerData = *testContainerDataPointer;

    ASSERT_TRUE(!testContainerData.empty());
    ASSERT_TRUE(testContainerData == someDoubles);
}

TEST_F(RawDataContainerTest, testContainerWithSimpleStructData) {
    ByteArray::CreationInput simpleStructTestCreationInput = {
            "simple struct data",
            pEngine::util::UniqueIdentifier(),
            reinterpret_cast<unsigned char *>(&simpleTestData),
            static_cast<unsigned long>(sizeof(simpleTestData))
    };

    std::shared_ptr<ByteArray> simpleStructTestContainer;
    ASSERT_NO_THROW(simpleStructTestContainer = std::make_shared<ByteArray>(simpleStructTestCreationInput));

    auto *testContainerDataPointer = simpleStructTestContainer->getRawDataPointer<SimpleTestData>();
    SimpleTestData testContainerData = *testContainerDataPointer;

    ASSERT_TRUE(testContainerData == simpleTestData);

    std::vector<SimpleTestData> simpleTestDataVector(4);

    std::shared_ptr<ByteArray> vectorOfSimpleDataStructContainer = std::make_shared<ByteArray>(
            ByteArray::CreationInput{
                    "simple vector of simple test data :)",
                    pEngine::util::UniqueIdentifier(),
                    reinterpret_cast<unsigned char *>(&simpleTestDataVector),
                    static_cast<unsigned long>(
                            sizeof(SimpleTestData) * simpleTestDataVector.size()
                    )
            });

    auto *testVectorContainerDataPointer = vectorOfSimpleDataStructContainer->getRawDataPointer<std::vector<SimpleTestData>>();
    auto testVectorContainerData = *testVectorContainerDataPointer;
    ASSERT_EQ(testVectorContainerData, simpleTestDataVector);
}

TEST_F(RawDataContainerTest, testSettingEmptyContainerToSimpleData) {
    ByteArray::CreationInput simpleStructTestCreationInput = {
            "empty simple string test data",
            pEngine::util::UniqueIdentifier(),
            nullptr,
            0
    };

    std::shared_ptr<ByteArray> simpleStructTestContainer;
    ASSERT_NO_THROW(simpleStructTestContainer = std::make_shared<ByteArray>(simpleStructTestCreationInput));
    ASSERT_NO_THROW(
            simpleStructTestContainer->setRawData(
                    reinterpret_cast<unsigned char *>(&simpleTestData),
                    sizeof(SimpleTestData))
    );

    ASSERT_TRUE(simpleStructTestContainer->getRawDataSizeInBytes() == sizeof(SimpleTestData));
    ASSERT_TRUE(*simpleStructTestContainer->getRawDataPointer<SimpleTestData>() == simpleTestData);
}

TEST_F(RawDataContainerTest, testContainerWithVertexData) {
    // TODO
}

TEST_F(RawDataContainerTest, testContainerWithIndexData) {
    // TODO
}
