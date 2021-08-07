#include "../../../include/core/thread_pool/ThreadPool.hpp"

#include "../../../include/core/thread_pool/job_queue/JobQueue.hpp"

// ThreadPool::ThreadPool(OSInterfacePtr osPtr, unsigned int size) : os_(osPtr), threadCount_(size) {
//     // get number of threads 
//     // threadCount_ = std::thread::hardware_concurrency();

//     // should probably create all the threads here in the constructor
//     // create threads on the heap (should be fine since they should exist for the entire lifetime of the program)
//     for (int i = 0; i < threadCount_; i++) {
//         threads_.push_back(new Thread(os_, &alive_, i));
//         // threads_[i]->setThreadIndex(i); // set thread index within the thread (why isn't this in the ctor lol)
//     }
// }

// will probably be useful to have the ability to create multiple thread pools 
    // eg if you want to split threads into groups, might be worth investigating
// overall seems easier to achieve that behavior by just assigning groups of threads to separate queues, so maybe not...
ThreadPool::ThreadPool(unsigned int size) : threadCount_(size) {
    // need to not duplicate this code as much
    // threads_.reserve(threadCount_);

    // create threads
    for (int i = 0; i < threadCount_; i++) {
        threads_.push_back(new Thread(&alive_, i));
    }

}

ThreadPool::~ThreadPool() {
    // will probably need to terminate the worker threads at this point, should only happen once engine is being destroyed

    // might have to wait on each thread ?
        // this will probably have to work differently
    // for (auto thread : threads_) {
    //     thread->finish(); // this should just join the std::thread, which should hopefully exit.. if it doesn't the program waits forever
    // }
    terminateWorkerThreads();

    // destroy any thread pool components.. not sure what is needed

}

/* Thread Pool Interface */

void ThreadPool::terminateWorkerThreads() {
    alive_ = false;

    // the threads should all die, so maybe just join them?
    for (auto thread : threads_) {
        thread->finish();
    }
        // seems to work for now
}

// void ThreadPool::registerJobQueue(std::shared_ptr<JobQueue> queue, const std::string &name, unsigned int priority) {
//     // i think the idea is that when we register a queue using this function, we have to enable it before threads will take 
//     // work from it

//     // enabling/disabling queues will have to be handled outside of the actual JobQueue to prevent the need for synchronization
//         // i think if the thread pool just has to grab each thread's lock it shouldn't be too bad as long as the threads give up control often enough
    
//     // i'm hoping i can just use unsigned int; it will allow the programmer to group things up by priority however they want,
//     // although honestly it would probably be better to just finish the critical workload and then start any lower priority work
//     // (ie use separate job queues for separate priority levels and complete their workloads sequentially)
    
//    if (registeredQueues_.count(name) != 0) {
//        throw std::runtime_error("Queue with name " + name + " already registered!");
//    }

//     // register the queue itself by name
//     registeredQueues_[name] = queue;

//     // register the queue in the ordered priorities map 
//     registeredQueuePriorities_[priority] = name;

//     // this function doesn't enable the queue 
// }

// void ThreadPool::registerExclusiveJobQueue(std::shared_ptr<JobQueue> queue, const std::string &name, unsigned int priority, unsigned int numThreads) {
//     registerJobQueue(queue, name, priority);

//     // activate the queue only for some number of threads
//         // gotta think about the best way to do this... do i track thread status (wrt queues) in the thread pool? 
//     // for now, i'll think i'll keep it kinda simplistic
//         // NOTE: might need to synchronize here (add a lock for threadpool data)
    
//     if (numExclusiveQueues_ >= MAX_DEDICATED_QUEUES) { // awful, hideous, etc. but for now it's that
//         // emit some kind of warning.... generally just handle this case lol
//         return;
//     }

//     numExclusiveQueues_++;
//     unsigned int counter = 0;
//     for (auto &thread : threads_) {
//         // have to get data lock on thread...
//         if (!thread->hasExclusiveQueue()) {
//             counter++;
//             thread->disableAllQueues();
//             thread->enableQueue(queue, name, priority);

//             if (counter >= numThreads) break;
//         }
//     }
// }

void ThreadPool::submitQueue(JobQueue *queue, unsigned int priority) {

    // i think we can just enable the queue with each thread...
    for (uint32_t i = 0; i < threads_.size(); i++) {
        // threads_[i]->enableQueue(std::move(queue), "");
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
    
    // 
}

/* Thread Pool Internal Functions */

