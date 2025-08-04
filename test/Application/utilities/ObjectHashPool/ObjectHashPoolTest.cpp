//
// Created by paull on 2023-11-27.
//

#include <gtest/gtest.h>

#include <glm/vec3.hpp>
#include "../../../../src/utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../../../src/utilities/ObjectHashPool/ObjectHashPool.hpp"

using namespace pEngine;

class ObjectHashPoolTest : public ::testing::Test {
protected:
    static constexpr unsigned long TEST_UPDATES_BEFORE_FREE = 8;

    void SetUp() override {
    }

    struct AllocatedObjectType : public util::objectPool::ObjectHashPoolEntry<AllocatedObjectType> {
        glm::vec3 aVector;
        glm::vec3 bVector;
        glm::vec3 cVector;

        AllocatedObjectType(const AllocatedObjectType &other) = default;

        AllocatedObjectType() : aVector(glm::vec3(0)), bVector(glm::vec3(0)), cVector(glm::vec3(0)) {
        }

        AllocatedObjectType(const float &a, const float &b, const float &c)
                : aVector(glm::vec3(a)),
                  bVector(glm::vec3(b)),
                  cVector(glm::vec3(c)) {
        }

        AllocatedObjectType &operator=(const AllocatedObjectType &other) = default;

        bool operator==(const AllocatedObjectType &rhs) const {
            return aVector == rhs.aVector && bVector == rhs.bVector && cVector == rhs.cVector;
        }

        bool operator!=(const AllocatedObjectType &rhs) const {
            return !(rhs == *this);
        }
    };

    std::shared_ptr<util::objectPool::ObjectHashPool<AllocatedObjectType, TEST_UPDATES_BEFORE_FREE> > hashPool =
            nullptr;
};

TEST_F(ObjectHashPoolTest, TestObjectAllocationAndRecycling) {
    hashPool = std::make_shared<util::objectPool::ObjectHashPool<AllocatedObjectType,
            TEST_UPDATES_BEFORE_FREE> >(
            util::objectPool::ObjectHashPool<AllocatedObjectType, TEST_UPDATES_BEFORE_FREE>
            ::CreationInput{
                    "testHashPool",
                    util::UniqueIdentifier()
            });

    std::string testObjectName = "my_new_object_string";
    size_t objectHash = std::hash<std::string>()(testObjectName);
    auto requestedObject =
            hashPool->requestObject(objectHash);
    ASSERT_EQ(requestedObject.second, util::objectPool::ReturnedObjectType::FRESH_ALLOCATION);
    ASSERT_EQ(requestedObject.first, AllocatedObjectType());

    // make a change to the object
    // alright, so at this point the mapValues of the allocationMap has a null value; why does this damn iterator keep going null?
    auto newAVectorValue = glm::vec3(1.0f);
    requestedObject.first.aVector = newAVectorValue;

    // test that when we request with the same key we get the same object (while the object hasn't been freed for too many updates without use)
    auto sameRequestedObject = hashPool->requestObject(objectHash);
    ASSERT_EQ(sameRequestedObject.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
    ASSERT_EQ(sameRequestedObject.first.aVector, newAVectorValue);
    ASSERT_EQ(sameRequestedObject.first, requestedObject.first);

    // test that a different key returns a different object
    std::string differentObjectName = "zibbidy-doo";
    size_t differentObjectHash = std::hash<std::string>()(differentObjectName);
    auto differentRequestedObject = hashPool->requestObject(differentObjectHash);

    ASSERT_EQ(differentRequestedObject.second, util::objectPool::ReturnedObjectType::FRESH_ALLOCATION);
    ASSERT_NE(differentRequestedObject.first, sameRequestedObject.first);
    ASSERT_NE(differentRequestedObject.first, requestedObject.first);

    // update until all the objects are freed; then test that all the entries end up returning "freshly allocated"
    for (int i = 0; i < TEST_UPDATES_BEFORE_FREE + 1; i++) {
        hashPool->update();
    }
    requestedObject = hashPool->requestObject(objectHash);
    differentRequestedObject = hashPool->requestObject(differentObjectHash);
    ASSERT_NE(requestedObject.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
    ASSERT_NE(differentRequestedObject.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);

    // update the hash pool enough times to trigger freeing of unused updates, and also
    // test that accessing one object will ensure it doesn't get freed in between updates
    for (int i = 0; i < TEST_UPDATES_BEFORE_FREE - 1; i++) {
        ASSERT_EQ(hashPool->requestObject(objectHash).second,
                  util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
        hashPool->update();
        // repeatedly access the first object after updating so the counter gets reset
    }
    hashPool->update(); // update once more to trigger the first requested object's being freed in the pool

    requestedObject = hashPool->requestObject(objectHash);
    differentRequestedObject = hashPool->requestObject(differentObjectHash);
    // recently accessed object should not have been allocated fresh
    ASSERT_EQ(requestedObject.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
    // object which hasn't been accessed should be allocated fresh (since the original allocated was freed)
    ASSERT_NE(differentRequestedObject.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
}

TEST_F(ObjectHashPoolTest, TestBreakingCases) {
    // idea here is to just add one more test in where we try to break the pool
    // and verify that such things are impossible.

    // there aren't any braindead ones I can think of off the top of my head but we'll see
}

TEST_F(ObjectHashPoolTest, StressTest) {
    // Should figure out how to make this test skippable; idea would be to have this do a whole bunch of spamming
    // requests and updates and printing the results... maybe I can come back to this later though

    constexpr int DEFAULT_NUM_UPDATES = 8;
    constexpr unsigned TEST_ITERATION_COUNT = 1000;

    // for now a simple test that just makes 1000 entries and updates a few times might be okay?
    hashPool = std::make_shared<util::objectPool::ObjectHashPool<AllocatedObjectType> >(
            util::objectPool::ObjectHashPool<AllocatedObjectType>::CreationInput{
                    "testPool",
                    util::UniqueIdentifier(),
            });


    // request 1000 objects from the start
    std::vector<util::Hash> keys(TEST_ITERATION_COUNT);
    for (int i = 0; i < keys.size(); i++) {
        keys[i] = std::hash<std::string>()("initialKey" + std::to_string(i));
    }

    auto beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATION_COUNT; i++) {
        const auto resultPair = hashPool->requestObject(keys[i]);
        ASSERT_EQ(resultPair.second, util::objectPool::ReturnedObjectType::FRESH_ALLOCATION);
    }
    auto afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    auto timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout << std::to_string(TEST_ITERATION_COUNT) + " fresh allocations with no updating - time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
              << "ms" << std::endl;

    // recycle 1000 objects without updating
    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATION_COUNT; i++) {
        ASSERT_NO_THROW(hashPool->requestObject(keys[i]));
    }
    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout << std::to_string(TEST_ITERATION_COUNT) + " recycled requests with no updating - time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
              << "ms" << std::endl;

    // modify keys
    for (int i = 0; i < keys.size(); i++) {
        keys[i] = std::hash<std::string>()("newKey" + std::to_string(i));
    }

    // request 1000 new objects
    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    for (unsigned long long key: keys) {
        const auto resultPair = hashPool->requestObject(key);
        hashPool->update();
        ASSERT_NE(resultPair.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
    }
    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout << std::to_string(TEST_ITERATION_COUNT) +
                 " fresh allocations with updates after every request - time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
              << "ms" << std::endl;

    // recycle 1000 objects while updating after every loop
    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    // request 1000 objects
    for (int i = 0; i < TEST_ITERATION_COUNT; i++) {
        ASSERT_NO_THROW(hashPool->requestObject(keys[i]));
        hashPool->update();
    }
    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout << std::to_string(TEST_ITERATION_COUNT) +
                 " recycled allocations with updates after every request - time elapsed: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
              << "ms" << std::endl;

    // recycle 1000 objects while updating after every loop and accessing previous keys
    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    // request 1000 objects
    for (int i = 0; i < TEST_ITERATION_COUNT; i++) {
        // access last 3 keys so that they last a bit longer in the pool
        if (i > DEFAULT_NUM_UPDATES) {
            for (int j = i - 3; j < i; j++) {
                const auto resultPair = hashPool->requestObject(keys[j]);
                ASSERT_EQ(resultPair.second, util::objectPool::ReturnedObjectType::REQUESTED_OBJECT);
            }
        }
        ASSERT_NO_THROW(hashPool->requestObject(keys[i]));
        hashPool->update();
    }
    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout
            << std::to_string(TEST_ITERATION_COUNT) +
               " recycled allocations with updates after every request and multiple accesses of previous keys - time elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
            << "ms" << std::endl;

    // one more test: less frequent updates
    unsigned updateCounter = 0;
    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    constexpr unsigned UPDATE_LIMIT = TEST_ITERATION_COUNT / 2;
    // request 1000 new objects
    for (int i = 0; i < TEST_ITERATION_COUNT; i++) {
        // access last 5 keys so that they last a bit longer in the pool
        if (i > DEFAULT_NUM_UPDATES) {
            for (int j = i - DEFAULT_NUM_UPDATES; j < i; j++) {
                ASSERT_NO_THROW(hashPool->requestObject(keys[j]));
            }
        }
        ASSERT_NO_THROW(hashPool->requestObject(keys[i]));
        if (updateCounter > UPDATE_LIMIT) {
            hashPool->update();
            updateCounter = 0;
        } else {
            updateCounter++;
        }
    }
    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
    std::cout
            << std::to_string(TEST_ITERATION_COUNT) + " recycled allocations with updates after every " +
               std::to_string(UPDATE_LIMIT) +
               " requests and multiple accesses of previous keys - time elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count()
            << "ms" << std::endl;
}
