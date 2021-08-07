#include "../../../../include/core/thread_pool/job_queue/JobQueue.hpp"

JobQueue::JobQueue(const std::string &name, ThreadPool &pool) : name_(name), pool_(pool) {

}

JobQueue::JobQueue(ThreadPool &pool) : pool_(pool) {

}

// JobQueue::JobQueue(JobQueue &copy) {
// }

JobQueue::~JobQueue() {
    
}

void JobQueue::pushPackagedJob(std::function<void()> job /*, Priority priority */) { 
    // this function should actually add a job to the given queue, 
    // and should hopefully wrap up all the synchronization required for a (presumably controller) thread to
    // acquire a queue and add a new job to it 
    // acquireLockBlock();
    std::unique_lock<std::mutex> ul(lock_); // TODO -> replace 2 calls to mutex with unique locks

    queue_.push_back(job);

    // releaseLock();
}

// this should deactivate and clear a queue (so should only be used as a way to halt all work from being completed in this queue)
void JobQueue::clear() {
    // not sure how useful this one will be 
    // acquireLockBlock();
    std::unique_lock<std::mutex> ul(lock_);

    // gonna use an empty-swap to clear the queue
    std::deque<std::function<void()>>().swap(queue_);

    // releaseLock();
}

bool JobQueue::isEmpty() {
    // acquireLockBlock();
    bool result = true;
    {
        std::unique_lock<std::mutex> ul(lock_); 
        result = queue_.empty();
    }

    // releaseLock();

    return result;
}

// bool JobQueue::isLocked() {
//     bool result = lock_.try_lock();
//     if (result) {
//         lock_.unlock(); // ugly: immediately unlock lol
//     }
// }

std::function<void()> JobQueue::checkForWork() {
    bool check = lock_.try_lock();
    if (!check) {
        return nullptr;
    }
    // if we acquired the lock, pop a job off the JobQueue's internal queue
    auto work = queue_.front();
    queue_.pop_front();

    // since we acquired the lock, we have to release it again
    releaseLock();

    return work;
}


// void JobQueue::activateQueue() {

// }

// void JobQueue::deactivateQueue() {

// }

void JobQueue::acquireLockBlock() {
    // ugh i could probably replace a lot of these with code blocks containing a unique_lock
    lock_.lock();
}

bool JobQueue::acquireLockNonblock() {
    if (lock_.try_lock()) {
        return true;
    }
    else {
        return false;
    }
}

void JobQueue::releaseLock() {
    lock_.unlock();
}

void JobQueue::flush() {
    // this should just execute all work in one burst by the calling thread
    std::unique_lock<std::mutex> ul(lock_);

    while (!queue_.empty()) {
        queue_[0]();
        queue_.pop_front();
    }
}

void JobQueue::submit() {
    std::unique_lock<std::mutex> ul(lock_);

    // acquiring lock -> can't modify the queue once you submit like this 
        // for ongoing/non-blocking work, the "enable/disable" queue functions should be good, since
        // they activate/deactivate queues without holding onto them until they're done executing

    pool_.submitQueue(this);
}