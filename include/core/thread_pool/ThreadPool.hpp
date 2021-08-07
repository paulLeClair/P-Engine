#pragma once

// include Thread object
#include "./Thread.hpp"

// stdlib includes
#include <thread>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <functional>

#define MAX_DEDICATED_QUEUES 8

class JobQueue;

class ThreadPool {
    // this class should be as light as possible, just needs to handle the worker threads used by the engine
        // still need to figure out certain specifics, but i'm thinking it'll be better to rewrite later as opposed to
        // trying to divine what exactly i'm gonna need now
  public:
    ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    // ThreadPool(OSInterfacePtr osPtr);
    ~ThreadPool();

    /* Thread Pool Interface */
    void terminateWorkerThreads(); // destroys all worker threads (renders threadpool useless until recreated rn)

    void submitQueue(JobQueue *queue, unsigned int priority = 1);

    unsigned int getNumThreads() const {
        return threadCount_;
    }

  private:
    /* Thread Pool State */
    bool alive_ = true; // flag for destroying worker threads once execution is complete
    unsigned int threadCount_;
    unsigned int numExclusiveQueues_ = 0;

    // THREADS
    std::vector<Thread*> threads_;

    /* Registered Queues */
    std::vector<JobQueue*> registeredQueues_;
    std::map<unsigned int, unsigned int, std::greater<unsigned int>> registeredQueuePriorities_;
    
    void enableQueue(const std::string &name); // given an existing queue, activate it with all non-dedicated threads
    void enableQueueForThread(const std::string &name, int threadIndex);

};
