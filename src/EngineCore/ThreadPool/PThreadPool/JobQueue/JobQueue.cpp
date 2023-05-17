//
// Created by paull on 2022-07-25.
//

#include "JobQueue.hpp"

namespace PEngine {


    JobQueue::JobQueue(const JobQueue::CreationInput &info) : name(info.name) {
        // use a max_enum value, so we can iterate over all priorities even if the specific priorities change
        for (size_t priority = 0; priority < static_cast<size_t>( Job::Priority::MAX_ENUM ); priority++) {
            jobsByPriority.emplace(std::pair(static_cast<Job::Priority>( priority ), std::deque<Job>()));
        }

        for (auto &jobCreateInfo: info.initialJobCreateInfos) {
            addJob(jobCreateInfo);
        }
    }

    Job JobQueue::acquireJobForWorkerThread() {
        /**
         *
         */
        Job job;

        {
            std::unique_lock<std::mutex> uniqueLock(jobQueueLock);
            bool jobFound = false;
            for (auto iterator = jobsByPriority.begin(); iterator != jobsByPriority.end(); iterator++) {
                auto &jobs = (*iterator).second;
                if (!jobs.empty()) {
                    job = jobs.front();
                    jobs.pop_front();
                    jobFound = true;
                    break;
                }
            }

            if (!jobFound) {
                throw std::runtime_error("Error in JobQueue::acquireJobForWorkerThread() - no job found!");
            }
        }

        conditionVariable.notify_all();
        return job;
    }

    bool JobQueue::isEmpty() {
        std::unique_lock<std::mutex> uniqueLock(jobQueueLock);

        bool allQueuesAreEmpty = true;

        for (const auto &workQueue: jobsByPriority) {
            allQueuesAreEmpty &= workQueue.second.empty();
        }

        return allQueuesAreEmpty;
    }

    JobQueue::AddJobResult JobQueue::addJob(const Job::CreationInput &jobCreateInfo) {
        try {
            std::unique_lock<std::mutex> uniqueLock(jobQueueLock);
            jobsByPriority[jobCreateInfo.priority].push_back(Job(jobCreateInfo));
        } catch (std::exception &exception) {
            return JobQueue::AddJobResult::FAILURE;
        }

        return JobQueue::AddJobResult::SUCCESS;
    }

/**
 * This is kinda just supposed to empty out the queue on one thread, I imagine this would happen only under certain
 * circumstances
 * @return
 */
    JobQueue::FlushQueueOnSingleThreadResult JobQueue::flushQueueOnSingleThread() {
        if (isEmpty()) {
            return JobQueue::FlushQueueOnSingleThreadResult::SUCCESS;
        }

        std::unique_lock<std::mutex> uniqueLock(jobQueueLock);

        for (auto &jobQueueByPriority: jobsByPriority) {
            for (auto &job: jobQueueByPriority.second) {
                auto result = job();
                if (result != Job::ExecuteJobResult::SUCCESS) {
                    return JobQueue::FlushQueueOnSingleThreadResult::FAILURE;// TODO - add a specific enum value for this
                }
            }
        }

        return JobQueue::FlushQueueOnSingleThreadResult::SUCCESS;
    }


}// namespace PEngine