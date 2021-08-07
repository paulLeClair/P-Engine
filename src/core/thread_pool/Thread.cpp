#include "../../../include/core/thread_pool/Thread.hpp"

#include "../../../include/core/thread_pool/job_queue/JobQueue.hpp"

Thread::Thread(const bool &aliveFlag, int index) : keepRunning_(true), threadIndex_(index) { // ignoring external flag

}

Thread::~Thread() {

}

/* Thread Interface */

void Thread::finish() {
    std::unique_lock<std::mutex> UL(threadDataLock_);

    keepRunning_ = false;
}

/* Thread Internal Functions */
bool Thread::acquireDataLockNonblock() {
    if (threadDataLock_.try_lock()) {
        return true;
    }
    else {
        return false;
    }
}

void Thread::acquireDataLockBlock() {
    threadDataLock_.lock();

}

void Thread::releaseDataLock() {
    // release the Thread data lock
    threadDataLock_.unlock();
}

/* Worker Thread Functions */

void Thread::workerThreadRun() {
    // TODO: make the worker thread loop more robust

    // trying something else: hopefully this works better
    int sleepTimeInMs = 15; // try 15 ms sleep for now, not sure the best way to structure that though lol
        // i'm gonna have to do a bunch more research into structuring the worker thread function 
    while (keepRunning_ == true) {

        // first maybe the thread can wait on its condition variable, it will only stop waiting to acquire work from any active queues
        std::shared_ptr<JobQueue> queue;
        std::function<void()> job = nullptr; // the thread keeps a local job 
        { // provide a scope for the unique lock
            std::unique_lock<std::mutex> queueCheckUL(threadDataLock_);

            threadCondVar_.wait(queueCheckUL, [this]() {
                // lambda predicate function, should make the thread wait for any active queues to exist
                    // true -> stop waiting
                    // false -> keep waiting
                if (!keepRunning_) {
                    // stop running
                    return true; // shouldn't be executed because the thread should stop running at this point
                }

                if (activeQueues_.size() == 0) {
                    return false;
                }

                bool allQueuesEmpty = true;

                for (const auto &activeQueue : activeQueues_) {
                    allQueuesEmpty = allQueuesEmpty & activeQueue->isEmpty();
                }
                return !allQueuesEmpty; // if all queues are empty, keep waiting
            });

            // if we stopped because keepRunning_ == false, release the thread data lock and exit the worker thread loop (ie die)
            if (!keepRunning_) {
                // i think the thread-specific data will be destroyed externally
                break; // break the worker thread loop, which ends worker thread execution
            }
            else {
                // else we've acquired the thread data lock, and so we acquire a particular queue
                // according to the thread's own active queues (sorted by priority)
                for (auto &nextQueueByPriorityPair : activeQueuePriorities_) {
                    // const std::string &name = nextQueueByPriorityPair.second;
                    unsigned int index = nextQueueByPriorityPair.second;
                    auto nextQueue = activeQueues_[index];

                    auto queryJob = nextQueue->checkForWork();
                    if (queryJob == nullptr) {
                        // if the job is null, the queue lock was unavailable and there is nothing to execute, so check any other queues 
                        // or try again
                        continue;
                    }
                    else {
                        // we acquired a job from the queue, so store it and relinquish the unique_lock by breaking the final loop
                        job = queryJob;
                        break;
                    }
                }
            }
        }

        // the thread has at least tried to acquire work from a queue (if none were available it will have just completed the cycle without accessing a queue)
            // the thread determines whether it has a job by just checking it's nullptr 
        if (job != nullptr) job();
    }
}

void Thread::enableQueue(JobQueue *queue, unsigned int priority) {
    std::unique_lock<std::mutex> ul(threadDataLock_);

    activeQueues_.push_back(queue);
    activeQueuePriorities_[activeQueues_.size() - 1] = priority;
}

void Thread::enableExclusiveQueue(JobQueue *queue) {
    std::unique_lock<std::mutex> ul(threadDataLock_);

    // hopefully this is safe
    activeQueues_.clear();
    activeQueuePriorities_.clear();

    activeQueues_.push_back(queue);
    activeQueuePriorities_[0] = 55; // why not schfifty five
}

void Thread::disableAllQueues() {
    std::unique_lock<std::mutex> ul(threadDataLock_);

    activeQueues_.clear();
    activeQueuePriorities_.clear();
}

bool Thread::hasExclusiveQueue() {
    bool result = false;
    {
        std::unique_lock<std::mutex> ul(threadDataLock_);
        result = hasExclusiveQueue_;        
    }

    return result;
}