//
// Created by paull on 2023-12-17.
//

#include <gtest/gtest.h>

#include "../../../../src/utilities/IntrusiveList/IntrusiveList.hpp"

using namespace pEngine::util;

struct TestNodeType : public IntrusiveListNode<TestNodeType> {
    unsigned data = 0u;
};

class IntrusiveListTest : public ::testing::Test {
protected:
    std::shared_ptr<IntrusiveList<TestNodeType>> testList = nullptr;
};

TEST_F(IntrusiveListTest, TestEmptyList) {
    testList = std::make_unique<IntrusiveList<TestNodeType>>();
    ASSERT_TRUE(testList->empty());
    ASSERT_TRUE(testList->begin().get() == nullptr);
    ASSERT_TRUE(testList->end().get() == nullptr);
}

TEST_F(IntrusiveListTest, TestSimpleList) {
    // I'll break this one up into a few smaller ones but first things first I'll throw everything in this test lol
    testList = std::make_unique<IntrusiveList<TestNodeType>>();
    TestNodeType testNode1 = TestNodeType();
    TestNodeType testNode2 = TestNodeType();
    testNode2.data = 1;
    TestNodeType testNode3 = TestNodeType();
    testNode3.data = 2;

    // insert first at front then last two at back
    ASSERT_NO_THROW(testList->insertFront(
            IntrusiveList<TestNodeType>::Iterator(&testNode1))
    );
    ASSERT_NO_THROW(testList->insertBack(
            IntrusiveList<TestNodeType>::Iterator(&testNode2))
    );
    ASSERT_NO_THROW(testList->insertBack(
            IntrusiveList<TestNodeType>::Iterator(&testNode3))
    );

    // assert that the list element datas line up as we expect
    ASSERT_FALSE(testList->empty());
    ASSERT_EQ(testList->begin().get()->data, testNode1.data);
    ASSERT_EQ((++testList->begin()).get()->data, testNode2.data);
    ASSERT_EQ((++(++testList->begin())).get()->data, testNode3.data);

    // clear the list and test insert front
    ASSERT_NO_THROW(testList->clear());
    ASSERT_TRUE(testList->empty());

    ASSERT_NO_THROW(testList->insertFront(
            IntrusiveList<TestNodeType>::Iterator(&testNode1))
    );
    ASSERT_NO_THROW(testList->insertFront(
            IntrusiveList<TestNodeType>::Iterator(&testNode2))
    );
    ASSERT_NO_THROW(testList->insertFront(
            IntrusiveList<TestNodeType>::Iterator(&testNode3))
    );

    ASSERT_FALSE(testList->empty());
    ASSERT_EQ(testList->begin().get()->data, testNode3.data);
    // access second element by incrementing iterator
    ASSERT_EQ((++testList->begin()).get()->data, testNode2.data);
    ASSERT_EQ((++(++testList->begin())).get()->data, testNode1.data);

    // TODO - test popFront() and popBack()
}

TEST_F(IntrusiveListTest, IteratorTestSuite) {
    // TODO - test that iterators for intrusive lists work properly
}

// TODO - any other tests!