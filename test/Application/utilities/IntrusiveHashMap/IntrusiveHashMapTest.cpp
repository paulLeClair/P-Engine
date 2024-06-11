//
// Created by paull on 2023-12-18.
//

#include <gtest/gtest.h>

#include <utility>

#include "../../../../src/utilities/IntrusiveHashMap/IntrusiveHashMap.hpp"

using namespace pEngine;

struct TestData : public pEngine::util::IntrusiveHashMapEntry<TestData> {
    explicit TestData(std::string label) : label(std::move(label)) {
    }

    TestData(const TestData &other) : label(other.label) {
        // NOLINT
    }

    TestData() = default;

    TestData &operator=(const TestData &other) {
        label = other.label;
        return *this;
    }

    bool operator==(const TestData &rhs) const {
        return label == rhs.label;
    }

    bool operator!=(const TestData &rhs) const {
        return !(rhs == *this);
    }

    std::string label;
};

using namespace pEngine;

class IntrusiveHashMapTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    std::shared_ptr<util::IntrusiveHashMap<TestData> > testHashMap = nullptr;
};

TEST_F(IntrusiveHashMapTest, EmptyMapTestSuite) {
    ASSERT_NO_THROW(testHashMap = std::make_shared<util::IntrusiveHashMap<TestData>>());
    ASSERT_TRUE(testHashMap->empty());
    ASSERT_TRUE(
        testHashMap->find(util::Hash(100)) == nullptr); // arbitrary hash lookup should return nullptr on empty list
    ASSERT_NO_THROW(testHashMap->erase(100));
}

TEST_F(IntrusiveHashMapTest, SimpleTestSuite) {
    // TODO - before fixing the object pool test, fix this shit!
    ASSERT_NO_THROW(testHashMap = std::make_shared<util::IntrusiveHashMap<TestData>>());

    TestData testData1 = TestData("label");
    TestData testData2 = TestData("label2");
    TestData testData3 = TestData("label3");

    // try pre-initializing the hashes (although I think it does this for you inside the insert*() functions
    // for this test struct, we compute the hash of the single string member
    testData1.setHash(std::hash<std::string>()(testData1.label));
    testData2.setHash(std::hash<std::string>()(testData2.label));
    testData3.setHash(std::hash<std::string>()(testData3.label));

    // test insertYield()
    // PROBLEM: when we add a new element, the last one gets its hash reset to zero... oops
    ASSERT_NO_THROW(testHashMap->insertYield(testData1.getHash(), testData1));
    ASSERT_NO_THROW(testHashMap->insertYield(testData2.getHash(), testData2));
    ASSERT_NO_THROW(testHashMap->insertYield(testData3.getHash(), testData3));

    ASSERT_EQ(*(testHashMap->find(testData1.getHash())), testData1);
    ASSERT_EQ(*(testHashMap->find(testData2.getHash())), testData2);
    ASSERT_EQ(*(testHashMap->find(testData3.getHash())), testData3);

    // test erasing elements from the list
    testHashMap->erase(testData1.getHash());
    testHashMap->erase(testData2.getHash());
    testHashMap->erase(testData3.getHash());

    ASSERT_EQ(testHashMap->find(testData1.getHash()), nullptr);
    ASSERT_EQ(testHashMap->find(testData2.getHash()), nullptr);
    ASSERT_EQ(testHashMap->find(testData3.getHash()), nullptr);

    // test insertReplace (including that it actually replaces entries)
    ASSERT_NO_THROW(testHashMap->insertReplace(testData1.getHash(), testData1));
    ASSERT_EQ(*testHashMap->find(testData1.getHash()), testData1);
    ASSERT_NO_THROW(testHashMap->insertReplace(testData1.getHash(), testData2));
    ASSERT_EQ(*testHashMap->find(testData1.getHash()), testData2);
}

TEST_F(IntrusiveHashMapTest, PODWrappedHashMapEntriesTestSuite) {
    ASSERT_NO_THROW(testHashMap = std::make_shared<util::IntrusiveHashMap<TestData>>());

    TestData testData1 = TestData("label");
    TestData testData2 = TestData("label2");
    TestData testData3 = TestData("label3");

    // try pre-initializing the hashes (although I think it does this for you inside the insert*() functions
    // for this test struct, we compute the hash of the single string member
    testData1.setHash(util::Hash(std::hash<std::string>()(testData1.label)));
    testData2.setHash(util::Hash(std::hash<std::string>()(testData2.label)));
    testData3.setHash(util::Hash(std::hash<std::string>()(testData3.label)));

    //TODO
}
