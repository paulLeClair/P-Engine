#include "../../../include/core/thread_pool/Thread.hpp"

#include "../../../include/core/thread_pool/job_queue/JobQueue.hpp"

Thread::Thread(const bool &aliveFlag, int index) : keepRunning_(true), threadIndex_(index) { // ignoring external flag

}

Thread::~Thread() {

}

/* Thread Interface */

void Thread::finish() {
    // if (thr_.joinable()) {
    //     thr_.join();
    // }
        // mar19 - ignoring it

    // new functionality:
    // acquireDataLockBlock();
    std::unique_lock<std::mutex> UL(threadDataLock_);

    keepRunning_ = false;

    // releaseDataLock();

    // NOW join the thread...?
    // if (thr_.joinable()) {
    //     thr_.join();
    // }

    // what if we just dont join...
        // this seems to fix it, not joining here anymore

}

/* Thread Internal Functions */
// i'm not sure exactly when this will be happen
bool Thread::acquireDataLockNonblock() {
    // std::unique_lock<std::mutex> threadDataLock(threadLock_);
    // if (threadCondVar_.wait_for(threadDataLock, std::chrono::milliseconds(0)) == std::cv_status::timeout) {
    //     // i think if the condvar is in timeout state, then nothing else awoke it? i'm not entirely sure
    // }

    // wait not sure i even need all that condvar stuff... maybe just use mutex::try_lock()? 
    if (threadDataLock_.try_lock()) {
        return true;
    }
    else {
        return false;
    }
}

void Thread::acquireDataLockBlock() {
    // std::unique_lock<std::mutex> threadDataLock(threadLock_);
    // threadCondVar_.wait(threadDataLock);

    // is there a blocking mutex lock() function? look into that
    threadDataLock_.lock();

}

void Thread::releaseDataLock() {
    // release the Thread data lock (not sure exactly how all this will work... but its kool)
    threadDataLock_.unlock();
}

/* Worker Thread Functions */

void Thread::workerThreadRun() {
    // not sure what threads should do right off the bat

    // honestly, could just involve doing any initial setup and then waiting to be assigned to a queue

    // for now, just have the thread exit upon creation
    // workerThreadExit();

    // trying something else: hopefully this works better
    int sleepTimeInMs = 50; // try 50 ms sleep for now, not sure the best way to structure that though lol
        // i'm gonna have to do a bunch more research into structuring the worker thread function 
    while (keepRunning_ == true) {
        // now have to make the threads do work...
            // maybe i'll leave this til after i do some work on the Vulkan side (as of boxing day 2020)
        
        // threads need to check some collection of registered queues

        // wait if no work is available (should be done with a condition variable)
        // std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeInMs));

        //MAR18 - now i gotta start rewriting this worker thread function
            // the thread pool manages this Thread object, which needs a way to know which job queues to search
        // this is where the priority issue comes in again... 

        // i think since each JobQueue contains its own synch variables, the worker threads can use some "check()" function here maybe

        // it might make sense to do the ordered map after all and NOT use an enum for queue priority, but just an unsigned int 
            // then this can just use a "for (auto queue : queuesMap_)" which should be already sorted, and the thread can do a safe check
            // on the condition variable/lock corresponding to the queue

        // since threads should be able to take work from multiple queues, when checking a queue they shouldn't wait for work and
        // threads need to sleep when all their active queues are empty

        // to track which queues are active, each thread could simply keep a std::string->flag unordered map for each queue, that way 
        // each thread can have its own config and the threadpool can control them using that
            // we could have it map to an enum of possible activity levels, including regular "enabled/disabled" options for general worker threads and
            // also an "exclusive" flag for having a thread be dedicated to a smaller subset of queues 

        /* THREAD LOGIC */
        // an issue is synchronizing between the controlling thread and the worker thread... i need to think about when the thread can allow the controlling thread
        // to make changes;
            // maybe the thread should wait on its own condition variable in an inner loop, and it'll only acquire its own lock when 

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
                // for (const auto &activeQueuePair : activeQueues_) {
                //     allQueuesEmpty = allQueuesEmpty & activeQueuePair.second->isEmpty(); // AND-> if the queue isn't empty, allQueuesEmpty becomes false 
                //                                                             // and false & true = false 
                // }
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

                    // what if i had a function that basically does a "try_lock", and if the function
                    // succeeds in acquiring the lock it returns the job, or else it returns nullptr?
                        // key idea: no matter what, release the lock after calling this 
                            // implmt'd by JobQueue::checkForWork()
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

// void Thread::enableQueue(std::shared_ptr<JobQueue> queue, std::string name, unsigned int priority) {
//     // acquireDataLockBlock();
//     std::unique_lock<std::mutex> ul(threadDataLock_);

//     activeQueues_[name] = queue;
//     activeQueuePriorities_[priority] = name;

//     // releaseDataLock();
// }

void Thread::enableQueue(JobQueue *queue, unsigned int priority) {
    std::unique_lock<std::mutex> ul(threadDataLock_);

    activeQueues_.push_back(queue);
    activeQueuePriorities_[activeQueues_.size() - 1] = priority;
}

// void Thread::enableExclusiveQueue(std::shared_ptr<JobQueue> queue, std::string name) {
//     // acquireDataLockBlock();
//     std::unique_lock<std::mutex> ul(threadDataLock_);

//     hasExclusiveQueue_ = true;

//     // i guess for now just have threads clear out all queues but the dedicated queue
//     activeQueues_.clear();
//     activeQueuePriorities_.clear();

//     activeQueues_[name] = queue;
//     // activeQueues_.push_back(queue);
//     activeQueuePriorities_[1u] = name; // just set to a default priority value for now since right now you can only exclusive assign to 1 queue

//     // releaseDataLock();
// }

void Thread::enableExclusiveQueue(JobQueue *queue) {
    std::unique_lock<std::mutex> ul(threadDataLock_);

    // hopefully this is safe
    activeQueues_.clear();
    activeQueuePriorities_.clear();

    activeQueues_.push_back(queue);
    activeQueuePriorities_[0] = 55; // why not schfifty five
}

// APR25 - commenting this out for now 
// void Thread::disableQueue(std::string name) {
//     // acquireDataLockBlock();
//     std::unique_lock<std::mutex> ul(threadDataLock_);

//     if (activeQueues_.count(name) == 0) {
//         return;
//     }

//     activeQueues_.erase(name);
    
//     // i'll use an iterator i guess to remove the queue from the sorted priorities map
//     for (auto it = activeQueuePriorities_.begin(); it != activeQueuePriorities_.end(); it++) {
//         if (it->second == name) {
//             activeQueuePriorities_.erase(it);
//             // i think it should be able to exit now, since there should be no multiple copies of the same name
//             break;
//         }
//     }

//     // releaseDataLock();
// }

void Thread::disableAllQueues() {
    // acquireDataLockBlock();
    std::unique_lock<std::mutex> ul(threadDataLock_);

    activeQueues_.clear();
    activeQueuePriorities_.clear();

    // acquireDataLockNonblock();
}

bool Thread::hasExclusiveQueue() {

    bool result = false;

    {
        std::unique_lock<std::mutex> ul(threadDataLock_);

        // acquireDataLockBlock();
        result = hasExclusiveQueue_;
        // releaseDataLock();
    }

    return result;
}