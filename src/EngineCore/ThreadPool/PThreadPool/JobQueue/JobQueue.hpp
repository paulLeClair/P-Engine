//
// Created by paull on 2022-07-25.
//

#pragma once

#include <Windows.h>

#include <deque>
#include <map>
#include <mutex>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>

#include "../Job/Job.hpp"

namespace PEngine {

/**
 * This should correspond to a single queue of work that you want to submit to the worker threads. These are probably
 * the main thing the user will work with to actually use the thread pool
 */
    class JobQueue {
    public:
        static const unsigned int MAX_QUEUE_WAIT_TIME_MS = 100;

        struct CreationInput {
            std::string name;

            // TODO - actually allow the user to easily specify a bitmask of enabled threads (or find another convenient way to do that)

            std::vector<Job::CreationInput> initialJobCreateInfos = {};
        };

        explicit JobQueue(const CreationInput &info);

        JobQueue(const JobQueue &other)
                : jobQueueLock(std::mutex()),
                  conditionVariable(std::condition_variable()),
                  jobsByPriority(other.jobsByPriority) {

        };

        JobQueue &operator=(const JobQueue &other) {
            jobsByPriority = other.jobsByPriority;
            return *this;
        }

        ~JobQueue() = default;

        enum class AddJobResult {
            SUCCESS,
            FAILURE
        };

        AddJobResult addJob(const Job::CreationInput &jobCreateInfo);

        Job acquireJobForWorkerThread();// TODO - determine if arguments are needed here

        bool isEmpty();

        enum class WaitForQueueToEmptyResult {
            SUCCESS,
            FAILURE
        };

        WaitForQueueToEmptyResult waitForQueueToEmpty() {
            {
                std::unique_lock<std::mutex> uniqueLock(jobQueueLock);

                conditionVariable.wait(uniqueLock, [&]() {
                    // note: return true -> stop waiting, return false -> keep waiting
                    for (auto &priorityToJobQueue: jobsByPriority) {
                        if (!priorityToJobQueue.second.empty()) {
                            return false;
                        }
                    }
                    return true;
                });
            }

            if (!isEmpty()) {
                return WaitForQueueToEmptyResult::FAILURE;
            }

            return WaitForQueueToEmptyResult::SUCCESS;
        }

        enum class FlushQueueOnSingleThreadResult {
            SUCCESS,
            FAILURE
        };

        FlushQueueOnSingleThreadResult flushQueueOnSingleThread();

        const std::string &getName() {
            return name;
        }

        unsigned int getNumberOfJobs() {
            std::unique_lock<std::mutex> uniqueLock(jobQueueLock);
            unsigned int numberOfJobs = 0;
            for (auto &priorityToJobQueuePair: jobsByPriority) {
                numberOfJobs += priorityToJobQueuePair.second.size();
            }

            return numberOfJobs;
        }

    private:
        const std::string name;

        std::mutex jobQueueLock;

        std::condition_variable conditionVariable;

        std::map<Job::Priority, std::deque<Job>, std::greater<>> jobsByPriority;
    };

}// namespace PEngine