//
// Created by paull on 2022-07-25.
//

#pragma once

#include <vector>

#include "../ThreadPool.hpp"
#include "Thread/Thread.hpp"

namespace PEngine {
    
    class PThreadPool : public ThreadPool {
    public:
        struct CreationInput {
            unsigned int numberOfThreads;
        };

        explicit PThreadPool(const CreationInput &createInfo);

        ~PThreadPool();

        [[nodiscard]] unsigned int getNumberOfWorkerThreads() override {
            return threads.size();
        }

        enum class AddJobQueueResult {
            SUCCESS,
            FAILURE
        };

        AddJobQueueResult addJobQueue(const std::string &name, const JobQueue::CreationInput &jobQueueCreateInfo);

        std::shared_ptr<JobQueue> getJobQueue(const std::string &name) {
            if (jobQueuesByName.count(name) == 0) {
                return nullptr;
            }

            return jobQueuesByName.at(name);
        }

        enum class ActivateJobQueueResult {
            SUCCESS,
            FAILURE
        };

        ActivateJobQueueResult activateJobQueue(const std::string &name) {
            if (jobQueuesByName.count(name) == 0) {
                return ActivateJobQueueResult::FAILURE;
            }

            std::shared_ptr<PEngine::JobQueue> jobQueue = jobQueuesByName.at(name);

            for (auto &thread: threads) {
                if (thread->assignJobQueue(jobQueue) != Thread::AssignJobQueueResult::SUCCESS) {
                    return ActivateJobQueueResult::FAILURE;
                }
            }

            activeJobQueues.push_back(name);

            return ActivateJobQueueResult::SUCCESS;
        }

        enum class DeactivateJobQueueResult {
            SUCCESS,
            FAILURE
        };

        DeactivateJobQueueResult deactivateJobQueue(const std::string &name) {
            if (jobQueuesByName.count(name) == 0) {
                return DeactivateJobQueueResult::FAILURE;
            }

            std::shared_ptr<JobQueue> jobQueue = jobQueuesByName.at(name);

            for (auto &thread: threads) {
                if (thread->removeJobQueue(jobQueue) != PEngine::Thread::RemoveJobQueueResult::SUCCESS) {
                    return DeactivateJobQueueResult::FAILURE;
                }
            }

            auto itr = std::remove_if(activeJobQueues.begin(), activeJobQueues.end(),
                                      [&](const std::string &activeJobQueue) {
                                          return name == activeJobQueue;
                                      });

            return DeactivateJobQueueResult::SUCCESS;
        }

        enum class WaitForQueueToEmptyResult {
            SUCCESS,
            FAILURE
        };

        WaitForQueueToEmptyResult waitForQueueToEmpty(const std::string &queueName);

        enum class WaitForAllQueuesToEmptyResult {
            SUCCESS,
            FAILURE
        };

        WaitForAllQueuesToEmptyResult waitForAllQueuesToEmpty();

        bool allThreadsAreRunning();

        [[nodiscard]] const std::vector<std::shared_ptr<Thread>> &getThreads() const {
            return threads;
        }

    private:
        std::vector<std::shared_ptr<Thread>> threads;

        std::unordered_map<std::string, std::shared_ptr<JobQueue>> jobQueuesByName;

        std::vector<std::string> activeJobQueues;
    };

}// namespace PEngine
