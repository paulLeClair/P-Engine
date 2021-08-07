#pragma once

// will need to get started on this too, since i'm writing a multithreaded engine/renderer 

// include Thread object
#include "./Thread.hpp"

// include OS object (just cause it will prob be necessary lol)
// #include "../os_interface/OSInterface.hpp"

// stdlib includes
#include <thread>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <functional>

/* THREAD POOL DESIGN - research, etc
- some key design points:
    - should use the same number of threads as there are hardware threads on the system 
        - have to look up how to reliably get this, i think it's not that hard, might just have to be an OSInterface thing
    - job queues should implement JOB STEALING - when a thread is finished it should start on another pending job 

- apparently its a good idea to have the ability to use multiple queues for different priority levels of jobs
    - this makes sense, could make a simple array/vector/whatever of queues and the workers will pick from the higher-pri ones
    preferentially

- threads need to sleep properly too when there is no work 

- one big thing: how are worker threads managed by the thread pool specifically?
    - maybe an exit() function from within the worker thread code is unnecessary; the thread pool can control whether worker threads
    are active by having threads check the value of some shared ThreadPool member flag, and they shouldn't leave their core loop until
    the ThreadPool explicitly sets the flag to false
- that could make the actual worker thread behavior much simpler, since it could all live inside one function for the most part
    - the simpler the code is, the easier it will be to make it solid and less prone to failures


*/

/* IMPLEMENTATION 


*/

// ill stick a couple macros in here, eventually these will probably get put into the code
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

    // os pointer (might be needed idk)
    // OSInterfacePtr os_;

    // THREADS
    std::vector<Thread*> threads_;

    /* Registered Queues */
        // what if i use two maps like a weirdo:
            // one encodes the priority of the registered queues and is a sorted map from priority -> queue name
            // the other maps the name of the queue to a shared pointer to the actual job queue
        // so threads could just maintain a copy of this priority->name map, but they don't need the 
        // actual name->queue map
            // in addition, once they've acquired a task from a queue they should release their data lock so that
            // the threadpool can modify their data while they execute the job, allowing threads to be retooled b/w jobs hopefully

    // std::map<unsigned int, std::string, std::greater<unsigned int>> registeredQueuePriorities_;
    // std::unordered_map<std::string, std::shared_ptr<JobQueue>> registeredQueues_; 
        // APR25 - gonna replace these 2 maps with 1 vector and 1 map
    std::vector<JobQueue*> registeredQueues_;
    std::map<unsigned int, unsigned int, std::greater<unsigned int>> registeredQueuePriorities_;

    // i'm thinking the thread pool doesn't need to track the job queues object, they can be maintained by the components
      // that means the thread pool object should be fairly light
    // especially because the worker thread functions are gonna be in the Thread object
    

    // MAR17 - i'm thinking that the thread pool itself will serve as the central interface used by the engine/components
        // that means it'll need a big interface for adding job queues 
    // void registerJobQueue(std::shared_ptr<JobQueue> queue, const std::string &name, unsigned int priority = 1u);
    // void registerExclusiveJobQueue(std::shared_ptr<JobQueue> queue, const std::string &name, unsigned int priority = 1u, unsigned int numThreads = 1u); // assign some number of threads (if possible) 

    
    void enableQueue(const std::string &name); // given an existing queue, activate it with all non-dedicated threads
    void enableQueueForThread(const std::string &name, int threadIndex);

  protected:

};