//
// Created by paull on 2022-07-25.
//

#include "Thread.hpp"

namespace pEngine::threadPool {

/**
 * This should basically be the "base implementation" of a worker thread, and should accomplish all the goals
 * that the engine itself requires.
 *
 * I'll be modeling this (as with the entire thread pool) off of the awesome open source CPP11 thread pool by Atanas
 * Rusev and Ferat Ali!
 *
 *
 */
    void Thread::threadMain() {
        const auto &queryJobQueuesForWork = [&](Job &job) {
            std::unique_lock<std::mutex> uniqueLock(threadLock);

            std::shared_ptr<JobQueue> nextJobQueueToQueryForWork = nullptr;
            threadConditionVariable.wait(uniqueLock,
                                         [&]() {
                                             // note: return true -> stop waiting, return false -> keep waiting

                                             if (!threadMainShouldKeepRunning) {
                                                 return true;
                                             }

                                             for (const auto &jobQueuePointer: jobQueues) {
                                                 bool jobQueuePointerIsEmpty = jobQueuePointer->isEmpty();

                                                 if (!jobQueuePointerIsEmpty) {
                                                     nextJobQueueToQueryForWork = jobQueuePointer;
                                                     return true;
                                                 }
                                             }

                                             return false;
                                         });

            if (!threadMainShouldKeepRunning) {
                // this isn't ideal, we're just returning an empty job in this case, but the thread should stop running
                // if this flag is set to false anyway
                return;
            }

            // acquire work from first non-empty queue that we find
            job = nextJobQueueToQueryForWork->acquireJobForWorkerThread();
        };

        const auto &keepRunningThreadMain = [&]() {
            std::unique_lock<std::mutex> uniqueLock(threadLock);
            return threadMainShouldKeepRunning;
        };

        threadMainIsCurrentlyRunning = true;
        while (keepRunningThreadMain()) {
            try {
                Job job;
                queryJobQueuesForWork(job);

                {
                    std::unique_lock<std::mutex> uniqueLock(threadLock);
                    if (!threadMainShouldKeepRunning) {
                        threadMainIsCurrentlyRunning = false;
                        return;
                    }
                }

                auto jobResult = job(*this);
                if (jobResult != Job::ExecuteJobResult::SUCCESS) {
                    threadMainIsCurrentlyRunning = false;
                    return;
                }

            } catch (std::exception &exception) {
                // TODO - log
                threadMainIsCurrentlyRunning = false;
                return;
            }
        }

        threadMainIsCurrentlyRunning = false;
    }

    Thread::Thread(const Thread::CreateInfo &createInfo)
            : index(createInfo.threadIndex),
              threadMainShouldKeepRunning(true),
              threadMainIsCurrentlyRunning(false) {
        startThread();
    }

    Thread::StartThreadResult Thread::startThread() {
        try {
            std::unique_lock<std::mutex> uniqueLock(threadLock);

            thread = std::thread(
                    [&]() {
                        threadMain();
                    });
            threadMainIsCurrentlyRunning = true;
            threadMainShouldKeepRunning = true;
            return Thread::StartThreadResult::SUCCESS;
        } catch (std::exception &exception) {
            threadMainShouldKeepRunning = false;
            return Thread::StartThreadResult::FAILURE;
        }
    }

    Thread::AssignJobQueueResult Thread::assignJobQueue(std::shared_ptr<JobQueue> &jobQueue) {
        if (jobQueue == nullptr) {
            return Thread::AssignJobQueueResult::FAILURE;
        }

        try {
            std::unique_lock<std::mutex> uniqueLock(threadLock);

            jobQueues.push_back(jobQueue);
        } catch (const std::exception &exception) {
            return Thread::AssignJobQueueResult::FAILURE;
        }

        threadConditionVariable.notify_all();
        return Thread::AssignJobQueueResult::SUCCESS;
    }

    Thread::RemoveJobQueueResult Thread::removeJobQueue(std::shared_ptr<JobQueue> &jobQueue) {
        if (jobQueue == nullptr) {
            return Thread::RemoveJobQueueResult::FAILURE;
        }

        try {
            std::unique_lock<std::mutex> uniqueLock(threadLock);

            for (auto iterator = jobQueues.begin(); iterator != jobQueues.end(); iterator++) {
                if ((*iterator)->getName() == jobQueue->getName()) {
                    jobQueues.erase(iterator);
                    break;
                }
            }
        } catch (std::exception &exception) {
            return Thread::RemoveJobQueueResult::FAILURE;
        }

        return Thread::RemoveJobQueueResult::SUCCESS;
    }

    Thread::StopThreadResult Thread::stopThread() {
        {
            std::unique_lock<std::mutex> uniqueLock(threadLock);
            threadMainShouldKeepRunning = false;
        }

        threadConditionVariable.notify_all();

        if (thread.joinable()) {
            thread.join();
            return Thread::StopThreadResult::SUCCESS;
        }

        return Thread::StopThreadResult::FAILURE;// TODO - change what it does when the thread can't be joined
    }


}// namespace PEngine