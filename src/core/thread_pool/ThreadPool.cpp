#include "../../../include/core/thread_pool/ThreadPool.hpp"

#include "../../../include/core/thread_pool/job_queue/JobQueue.hpp"

// will probably be useful to have the ability to create multiple thread pools 
    // eg if you want to split threads into groups, might be worth investigating
// overall seems easier to achieve that behavior by just assigning groups of threads to separate queues, so maybe not...
ThreadPool::ThreadPool(unsigned int size) : threadCount_(size) {
    // create threads
    for (int i = 0; i < threadCount_; i++) {
        threads_.push_back(new Thread(&alive_, i));
    }

}

ThreadPool::~ThreadPool() {
    // will probably need to terminate the worker threads at this point, should only happen once engine is being destroyed

    terminateWorkerThreads();

}

/* Thread Pool Interface */

void ThreadPool::terminateWorkerThreads() {
    alive_ = false;

    // the threads should all die, so maybe just join them?
    for (auto thread : threads_) {
        thread->finish();
    }
}

void ThreadPool::submitQueue(JobQueue *queue, unsigned int priority) {
    // i might want to add more functions like this, such as for enabling a queue only on a subset of threads etc
    // i think we can just enable the queue with each thread...
    for (uint32_t i = 0; i < threads_.size(); i++) {
        threads_[i]->enableQueue(queue, priority); // maybe i should have threads work with raw pointers..
    }

    // after enabling queues, just wait for the queue to be empty i think
        // keeping it simpler for now...
    std::mutex submitLock;
    std::condition_variable cv;
    {
        std::unique_lock<std::mutex> ul(submitLock);

        cv.wait(ul, [&]() {
            // this function is thread-safe i believe
            return queue->isEmpty();
        });
    }
}

/* Thread Pool Internal Functions */

