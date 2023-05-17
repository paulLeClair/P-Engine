//
// Created by paull on 2022-08-05.
//

#include <gtest/gtest.h>
#include <Windows.h>

#include "../../../../../src/EngineCore/ThreadPool/PThreadPool/Thread/Thread.hpp"

#define TEST_VALUE_SET_BY_JOB 5

#define TEST_VALUE_UNSET 0

#define NUMBER_OF_TEST_VALUES 50

using namespace PEngine;

class ThreadTest : public ::testing::Test {
protected:
    void SetUp() override {
        testThread = std::make_shared<Thread>(Thread::CreateInfo{
                0
        });
    }

    std::shared_ptr<Thread> testThread;

};

TEST_F(ThreadTest, BasicCreation) {
    ASSERT_TRUE(testThread != nullptr);
    ASSERT_TRUE(testThread->isThreadRunning() && testThread->isJoinable());
}

TEST_F(ThreadTest, ThreadsCanBeStoppedSuccessfully) {
    ASSERT_TRUE(testThread->isThreadRunning() && testThread->isJoinable());
    ASSERT_TRUE(testThread->isThreadMainCurrentlyRunning());
    ASSERT_TRUE(testThread->stopThread() == Thread::StopThreadResult::SUCCESS);
    ASSERT_TRUE(!testThread->isThreadRunning() && !testThread->isJoinable());
    ASSERT_TRUE(!testThread->isThreadMainCurrentlyRunning());
}

TEST_F(ThreadTest, ThreadsCanBeStoppedAndRestarted) {
    ASSERT_TRUE(testThread->isThreadRunning() && testThread->isJoinable());
    ASSERT_TRUE(testThread->isThreadMainCurrentlyRunning());

    ASSERT_NO_THROW(testThread->stopThread());
    ASSERT_TRUE(!testThread->isThreadRunning() && !testThread->isJoinable());
    ASSERT_TRUE(!testThread->isThreadMainCurrentlyRunning());

    ASSERT_NO_THROW(testThread->startThread());
    ASSERT_TRUE(testThread->isThreadRunning() && testThread->isJoinable());
    ASSERT_TRUE(testThread->isThreadMainCurrentlyRunning());
}

TEST_F(ThreadTest, AssignJobQueueWithSingleJob) {
    int testValue = TEST_VALUE_UNSET;

    auto jobFunction = [&](const unsigned int &threadIndex) {
        testValue = TEST_VALUE_SET_BY_JOB;
    };
    Job::CreationInput jobCreationInput = {
            Job::Priority::HIGH,
            jobFunction
    };

    std::vector<Job::CreationInput> jobCreationInputs = {jobCreationInput};

    JobQueue::CreationInput jobQueueCreationInput = {
            "single job queue",
            jobCreationInputs
    };

    std::shared_ptr<JobQueue> jobQueue = std::make_shared<JobQueue>(jobQueueCreationInput);
    ASSERT_TRUE(testThread->assignJobQueue(jobQueue) == Thread::AssignJobQueueResult::SUCCESS);
    auto queueEmptyResult = jobQueue->waitForQueueToEmpty();
    ASSERT_TRUE(queueEmptyResult == JobQueue::WaitForQueueToEmptyResult::SUCCESS);
}

TEST_F(ThreadTest, AssignJobQueueWithNumerousJobs) {
    std::vector<int> testValues(NUMBER_OF_TEST_VALUES);

    std::vector<Job::CreationInput> jobCreationInputs = {};
    for (auto &testValue: testValues) {
        testValue = TEST_VALUE_UNSET;

        auto jobFunction = [&](const unsigned int &threadIndex) {
            testValue = TEST_VALUE_SET_BY_JOB;
        };

        Job::CreationInput jobCreationInput = {
                Job::Priority::HIGH,
                jobFunction
        };

        jobCreationInputs.push_back(jobCreationInput);
    }

    JobQueue::CreationInput jobQueueCreationInput = {
            "multiple jobs queue",
            jobCreationInputs
    };

    std::shared_ptr<JobQueue> jobQueue = std::make_shared<JobQueue>(jobQueueCreationInput);
    ASSERT_TRUE(testThread->assignJobQueue(jobQueue) == Thread::AssignJobQueueResult::SUCCESS);
    auto queueEmptyResult = jobQueue->waitForQueueToEmpty();
    ASSERT_TRUE(queueEmptyResult == JobQueue::WaitForQueueToEmptyResult::SUCCESS);
    ASSERT_TRUE(std::all_of(testValues.begin(), testValues.end(), [&](const int &testValue) {
        return testValue == TEST_VALUE_SET_BY_JOB;
    }));
}