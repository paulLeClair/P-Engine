#pragma once

// stdlib includes
#include <thread>
#include <iostream>
#include <mutex>
#include <stack>
#include <map>
#include <unordered_map>

class JobQueue;

// i'm thinking it's a good idea to encapsulate the std::thread in a Thread object
class Thread {
  public:
    Thread(const bool &aliveFlag, int index);
    ~Thread();

    /* Thread Interface */
    void enableQueue(JobQueue *queue, unsigned int priority = 1);
    void enableExclusiveQueue(JobQueue *queue);
    
    void disableAllQueues();
    bool hasExclusiveQueue(); // if the thread has been assigned to a single queue exclusively (for now that's the only simplistic mechanism)

    // finish() - join the thread and wait until it finishes
    void finish();

    int getIndex() const {
        return threadIndex_;
    }

  private:
    /* Thread State */
    // actual thread
    std::thread &thr_ = std::thread(&Thread::workerThreadRun, this);
    
    // just gonna store and set the thread index after creation
    int threadIndex_;

    // flag that determines whether thread keeps running (should come from ThreadPool)
    bool keepRunning_ = true;
    
    // quick bool for querying whether a thread has an exclusive thread assigned already
    bool hasExclusiveQueue_ = false;

    // synchronization stuff?
    // probably need a lock for accessing the Thread's data (since the ThreadPool will directly modify the data and the actual std::thread will
    // access the Thread's data, so there has to be synchronization there i believe
    std::mutex threadDataLock_;
    std::condition_variable threadCondVar_; // might have a use for this: since threads need to access their data but need to allow changes too, the
                                              // controlling threads might need 

    // i guess i'll try having the thread maintain its own maps
    std::vector<JobQueue*> activeQueues_;
    std::map<unsigned int, unsigned int, std::greater<unsigned int>> activeQueuePriorities_; // gonnna use indices instead of string

    /* Thread Internal Functions */

    // functions for locking/unlocking the thread data mutex
    bool acquireDataLockNonblock();
    void acquireDataLockBlock();
    void releaseDataLock();

    /* Worker Thread Functions */
    // this is where the abstract "FSM" of each worker thread will be implemented

    // should mostly consist of a few different states for workers to be in, where they 
    // somehow (still need to think about it)

    void workerThreadRun(); // maybe just a simple startup function for now
};
