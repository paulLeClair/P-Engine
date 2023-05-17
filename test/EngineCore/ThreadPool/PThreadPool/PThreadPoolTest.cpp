//
// Created by paull on 2022-08-05.
//

#include <gtest/gtest.h>

#include <Windows.h>

#include "../../../../src/EngineCore/ThreadPool/PThreadPool/PThreadPool.hpp"

static const unsigned int TEST_VALUE_UNSET = 0;
static const unsigned int TEST_VALUE_SET = 5;

static const int NUMBER_OF_TEST_JOBS_PER_WORKER_THREAD = 8000;

#define DEFAULT_NUMBER_OF_THREADS 4

using namespace PEngine;

class PThreadPoolTest : public ::testing::Test {
protected:
    PThreadPoolTest() {
        unsigned int hintNumberOfThreads = std::thread::hardware_concurrency() / 4;
        if (!hintNumberOfThreads) {
            hintNumberOfThreads = DEFAULT_NUMBER_OF_THREADS;
        }
        threadPool = std::make_shared<PThreadPool>(PThreadPool::CreationInput{
                hintNumberOfThreads
        });
    }

    std::shared_ptr<PThreadPool> threadPool = nullptr;
};


TEST_F(PThreadPoolTest, CreateThreadPool) {
    ASSERT_TRUE(std::thread::hardware_concurrency() != 0);

    PEngine::PThreadPool::CreationInput threadPoolCreateInfo = {
            std::thread::hardware_concurrency()
    };
    ASSERT_NO_THROW(PEngine::PThreadPool threadPool(threadPoolCreateInfo));
    PEngine::PThreadPool threadPool(threadPoolCreateInfo);

    ASSERT_TRUE(threadPool.getNumberOfWorkerThreads() != 0);
    ASSERT_TRUE(threadPool.getNumberOfWorkerThreads() == std::thread::hardware_concurrency());
    ASSERT_TRUE(threadPool.allThreadsAreRunning());
}

TEST_F(PThreadPoolTest, AddAndGetJobQueue) {
    ASSERT_TRUE(std::thread::hardware_concurrency() != 0);

    PEngine::JobQueue::CreationInput createInfo = {
            "testQueue",
            {}
    };
    std::string name = "testEmptyJobQueue";
    ASSERT_TRUE(threadPool->addJobQueue(name, createInfo) ==
                PEngine::PThreadPool::AddJobQueueResult::SUCCESS);

    ASSERT_TRUE(threadPool->getJobQueue(name) != nullptr);

    auto waitForQueueToEmptyResult = threadPool->getJobQueue(name)->waitForQueueToEmpty();
    ASSERT_EQ(waitForQueueToEmptyResult, PEngine::JobQueue::WaitForQueueToEmptyResult::SUCCESS);
}

TEST_F(PThreadPoolTest, TestJobsAreExecutedWhenQueueIsActivated) {
    unsigned int testDataSize = threadPool->getNumberOfWorkerThreads() * NUMBER_OF_TEST_JOBS_PER_WORKER_THREAD;
    std::vector<int> testData(testDataSize);

    for (auto &test: testData) {
        test = TEST_VALUE_UNSET;
    }

    std::vector<PEngine::Job::CreationInput> validJobCreateInfos = {};
    for (int &index: testData) {
        PEngine::Job::CreationInput validJobCreateInfo{
                PEngine::Job::Priority::HIGH,
                [&](const unsigned int &workerThread) {
                    index = TEST_VALUE_SET;
                }};
        validJobCreateInfos.push_back(validJobCreateInfo);
    }

    PEngine::JobQueue::CreationInput testJobQueueCreateInfo = {
            "testQueue",
            validJobCreateInfos
    };
    std::string name = "testJobQueue";
    ASSERT_TRUE(threadPool->addJobQueue(name, testJobQueueCreateInfo) ==
                PEngine::PThreadPool::AddJobQueueResult::SUCCESS);

    ASSERT_TRUE(threadPool->getJobQueue(name) != nullptr);

    ASSERT_EQ(threadPool->activateJobQueue(name), PEngine::PThreadPool::ActivateJobQueueResult::SUCCESS);

    auto waitForQueueToEmptyResult = threadPool->waitForQueueToEmpty(name);

    ASSERT_EQ(waitForQueueToEmptyResult, PEngine::PThreadPool::WaitForQueueToEmptyResult::SUCCESS);

    for (int testValue: testData) {
        ASSERT_EQ(testValue, TEST_VALUE_SET);
    }
}

// TODO - add a test for adding jobs **after** the queue is activated - we should be able to have continuously running queues like that

// TODO - add a test for having jobs that require synchronization and ensure they complete properly

