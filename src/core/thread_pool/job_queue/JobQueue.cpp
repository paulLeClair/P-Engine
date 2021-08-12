#include "../../../../include/core/thread_pool/job_queue/JobQueue.hpp"

JobQueue::JobQueue(const std::string &name, ThreadPool &pool) : name_(name), pool_(pool) {

}

JobQueue::JobQueue(ThreadPool &pool) : pool_(pool) {

}

JobQueue::~JobQueue() {
    
}

void JobQueue::pushPackagedJob(std::function<void()> job /*, Priority priority */) {
    std::unique_lock<std::mutex> ul(lock_);

    queue_.push_back(job);

}

// this should deactivate and clear a queue (so should only be used as a way to halt all work from being completed in this queue)
void JobQueue::clear() {
    std::unique_lock<std::mutex> ul(lock_);

    // gonna use an empty-swap to clear the queue
    std::deque<std::function<void()>>().swap(queue_);
}

bool JobQueue::isEmpty() {
    bool result = true;
    {
        std::unique_lock<std::mutex> ul(lock_); 
        result = queue_.empty();
    }

    return result;
}

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

void JobQueue::acquireLockBlock() {
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
    // this should just execute all work in one single-threaded burst by the calling thread
    std::unique_lock<std::mutex> ul(lock_);

    while (!queue_.empty()) {
        queue_[0]();
        queue_.pop_front();
    }
}

void JobQueue::submit() {
    // some parts of this JobQueue class are incomplete and need to be redone
    std::unique_lock<std::mutex> ul(lock_);
    pool_.submitQueue(this);
}
