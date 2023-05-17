//
// Created by paull on 2022-07-25.
//

#include "PThreadPool.hpp"

#include <algorithm>
#include <iostream>

namespace PEngine {

    PThreadPool::PThreadPool(const PThreadPool::CreationInput &createInfo) {
        for (unsigned long threadIndex = 0; threadIndex < createInfo.numberOfThreads; threadIndex++) {
            Thread::CreateInfo threadCreateInfo = {threadIndex};
            threads.push_back(std::make_shared<Thread>(threadCreateInfo));
        }
    }

    PThreadPool::AddJobQueueResult PThreadPool::addJobQueue(const std::string &name,
                                                            const JobQueue::CreationInput &jobQueueCreateInfo) {
        if (jobQueuesByName.count(name) != 0) {
            return PThreadPool::AddJobQueueResult::FAILURE;
        }

        std::shared_ptr<JobQueue> newJobQueue = std::make_shared<JobQueue>(jobQueueCreateInfo);

        jobQueuesByName[name] = newJobQueue;


        return PThreadPool::AddJobQueueResult::SUCCESS;
    }


    PThreadPool::~PThreadPool() {
        for (auto &thread: threads) {
            thread->stopThread();
        }
    }

    bool PThreadPool::allThreadsAreRunning() {
        bool allThreadsAreRunning = true;
        for (auto &thread: threads) {
            allThreadsAreRunning &= thread->isThreadRunning();
        }
        return allThreadsAreRunning;
    }

    PThreadPool::WaitForQueueToEmptyResult PThreadPool::waitForQueueToEmpty(const std::string &queueName) {
        const auto &queueIsActive = [&](const std::string &name) {
            return std::any_of(activeJobQueues.begin(), activeJobQueues.end(), [&](const std::string &activeJobQueue) {
                return activeJobQueue == queueName;
            });
        };

        if (jobQueuesByName.count(queueName) == 0 || !queueIsActive(queueName)) {
            return PThreadPool::WaitForQueueToEmptyResult::FAILURE;
        }

        auto jobQueue = jobQueuesByName[queueName];

        auto startTime = std::chrono::system_clock::now();
        auto waitUntilEmptyResult = jobQueue->waitForQueueToEmpty();
        if (waitUntilEmptyResult != JobQueue::WaitForQueueToEmptyResult::SUCCESS) {
            return PThreadPool::WaitForQueueToEmptyResult::FAILURE;
        }
        auto duration = std::chrono::system_clock::now() - startTime;
        auto durationInMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        std::cout << durationInMs.count() << " ms" << std::endl;

        deactivateJobQueue(queueName);

        return PThreadPool::WaitForQueueToEmptyResult::SUCCESS;
    }

    PThreadPool::WaitForAllQueuesToEmptyResult PThreadPool::waitForAllQueuesToEmpty() {
        return PThreadPool::WaitForAllQueuesToEmptyResult::FAILURE;
    }


}// namespace PEngine