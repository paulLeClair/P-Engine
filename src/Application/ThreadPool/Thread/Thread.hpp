//
// Created by paull on 2022-07-25.
//

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

//#include "../PThreadPool/JobQueue/JobQueue.hpp"

namespace pEngine::threadPool {

    class Thread {
    public:
        struct CreateInfo {
            unsigned int threadIndex;

            // std::function<void()> threadMain; // TODO - determine whether we want to enable user-supplied main
        };

        explicit Thread(const CreateInfo &createInfo);

        Thread(const Thread &other) = delete;

        Thread(const Thread &&other) = delete;

        ~Thread() {
            try {
                auto stopThreadResult = stopThread();
                if (stopThreadResult != StopThreadResult::SUCCESS) {
                    // TODO - log
                }
            }
            catch (std::exception &exception) {
                // TODO - log
            }
        }

        enum class StopThreadResult {
            SUCCESS,
            FAILURE
        };

        StopThreadResult stopThread();

        enum class StartThreadResult {
            SUCCESS,
            FAILURE
        };

        StartThreadResult startThread();

        enum class AssignJobQueueResult {
            SUCCESS,
            FAILURE
        };

        AssignJobQueueResult assignJobQueue(std::shared_ptr<JobQueue> &jobQueue);

        bool isThreadRunning() {
            std::unique_lock<std::mutex> uniqueLock(threadLock);
            return threadMainShouldKeepRunning && threadMainIsCurrentlyRunning && thread.joinable();
        }

        enum class RemoveJobQueueResult {
            SUCCESS,
            FAILURE
        };

        RemoveJobQueueResult removeJobQueue(std::shared_ptr<JobQueue> &jobQueue);

        [[nodiscard]] unsigned int getIndex() const {
            return index;
        }

        [[nodiscard]] bool isJoinable() const {
            return thread.joinable();
        }

        [[nodiscard]] bool isThreadMainCurrentlyRunning() {
            std::unique_lock<std::mutex> uniqueLock(threadLock);
            return threadMainIsCurrentlyRunning;
        }

    private:
        static const unsigned int THREAD_JOB_QUEUE_MAX_WAIT_TIME_MS = 1;

        std::mutex threadLock;

        std::condition_variable threadConditionVariable;

        bool threadMainShouldKeepRunning;
        bool threadMainIsCurrentlyRunning;

        const unsigned int index;

        std::thread thread;

        std::vector<std::shared_ptr<JobQueue>> jobQueues;

        void threadMain();
    };

}// namespace PEngine
