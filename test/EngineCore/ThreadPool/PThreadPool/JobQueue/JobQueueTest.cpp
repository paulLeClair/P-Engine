//
// Created by paull on 2022-08-05.
//

#include <gtest/gtest.h>
#include "../../../../../src/EngineCore/ThreadPool/PThreadPool/JobQueue/JobQueue.hpp"

using namespace PEngine;

#define TEST_VALUE_UNSET 0;

static const int TEST_VALUE_SET_BY_JOB = 5;

static const int LOW_PRIORITY_JOB_TEST_VALUE = 3;

static const int HIGH_PRIORITY_JOB_TEST_VALUE = 10;

static const char *const TEST_JOB_QUEUE_NAME = "valid test job queue";

class JobQueueTest : public ::testing::Test {
protected:
    JobQueueTest() {
        jobQueue = std::make_shared<JobQueue>(JobQueue::CreationInput{
                TEST_JOB_QUEUE_NAME,
                {}
        });
    }

    std::shared_ptr<JobQueue> jobQueue = nullptr;

    std::shared_ptr<JobQueue> uninitializedJobQueue = nullptr;

    inline void setupValidSimpleTestJobAndAddToQueue(int &testValue) {
        testValue = TEST_VALUE_UNSET;

        Job::CreationInput validJobCreateInfo = {
                PEngine::Job::Priority::HIGH,
                [&](const unsigned int &) {
                    testValue = TEST_VALUE_SET_BY_JOB;
                }
        };

        auto addJobResult = jobQueue->addJob(validJobCreateInfo);
        ASSERT_TRUE(addJobResult == JobQueue::AddJobResult::SUCCESS);
        ASSERT_TRUE(!jobQueue->isEmpty());
        ASSERT_TRUE(jobQueue->getNumberOfJobs() == 1);
    }

};

TEST_F(JobQueueTest, CreateEmptyJobQueue) {
    ASSERT_TRUE(jobQueue->getName() == TEST_JOB_QUEUE_NAME);
    ASSERT_TRUE(jobQueue->isEmpty());
}

TEST_F(JobQueueTest, AddValidJobToQueue) {
    Job::CreationInput validJobCreateInfo = {
            PEngine::Job::Priority::HIGH,
            [&](const unsigned int &) {
                // something valid
            }
    };

    auto result = jobQueue->addJob(validJobCreateInfo);
    ASSERT_TRUE(result == JobQueue::AddJobResult::SUCCESS);
    ASSERT_TRUE(!jobQueue->isEmpty());
    ASSERT_TRUE(jobQueue->getNumberOfJobs() == 1);
}

TEST_F(JobQueueTest, FlushValidJobFromQueue) {
    int testValue;

    setupValidSimpleTestJobAndAddToQueue(testValue);

    auto flushResult = jobQueue->flushQueueOnSingleThread();
    ASSERT_TRUE(flushResult == PEngine::JobQueue::FlushQueueOnSingleThreadResult::SUCCESS);
    ASSERT_TRUE(testValue == TEST_VALUE_SET_BY_JOB);
}

TEST_F(JobQueueTest, FlushFailingJobFromQueue) {
    Job::CreationInput validJobCreateInfo = {
            PEngine::Job::Priority::HIGH,
            [&](const unsigned int &) {
                throw std::runtime_error("Job threw an exception!!");
            }
    };

    auto addJobResult = jobQueue->addJob(validJobCreateInfo);
    ASSERT_TRUE(addJobResult == JobQueue::AddJobResult::SUCCESS);
    ASSERT_TRUE(!jobQueue->isEmpty());
    ASSERT_TRUE(jobQueue->getNumberOfJobs() == 1);

    auto flushResult = jobQueue->flushQueueOnSingleThread();
    ASSERT_TRUE(flushResult == PEngine::JobQueue::FlushQueueOnSingleThreadResult::FAILURE);
}

TEST_F(JobQueueTest, AddValidJobThenAcquireAndExecuteIt) {
    int testValue = TEST_VALUE_UNSET;

    setupValidSimpleTestJobAndAddToQueue(testValue);

    auto job = jobQueue->acquireJobForWorkerThread();
    auto result = job();
    ASSERT_TRUE(result == PEngine::Job::ExecuteJobResult::SUCCESS);
    ASSERT_TRUE(testValue == TEST_VALUE_SET_BY_JOB);
}

TEST_F(JobQueueTest, TestHigherPriorityJobsAreAcquiredFirst) {
    int testValue = TEST_VALUE_UNSET;
    Job::CreationInput lowPriorityJobCreateInfo = {
            Job::Priority::LOW,
            [&](const unsigned int &) {
                testValue = LOW_PRIORITY_JOB_TEST_VALUE;
            }
    };
    Job::CreationInput highPriorityJobCreateInfo = {
            Job::Priority::HIGH,
            [&](const unsigned int &) {
                testValue = HIGH_PRIORITY_JOB_TEST_VALUE;
            }
    };

    ASSERT_TRUE(jobQueue->addJob(lowPriorityJobCreateInfo) == JobQueue::AddJobResult::SUCCESS);
    ASSERT_TRUE(jobQueue->addJob(highPriorityJobCreateInfo) == JobQueue::AddJobResult::SUCCESS);


    auto job = jobQueue->acquireJobForWorkerThread();
    auto result = job();
    ASSERT_TRUE(result == Job::ExecuteJobResult::SUCCESS);
    ASSERT_TRUE(job.getPriority() == Job::Priority::HIGH);
    ASSERT_EQ(testValue, HIGH_PRIORITY_JOB_TEST_VALUE);

    job = jobQueue->acquireJobForWorkerThread();
    result = job();
    ASSERT_TRUE(result == PEngine::Job::ExecuteJobResult::SUCCESS);
    ASSERT_TRUE(job.getPriority() == PEngine::Job::Priority::LOW);
    ASSERT_EQ(testValue, LOW_PRIORITY_JOB_TEST_VALUE);
}