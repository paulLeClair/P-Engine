#pragma once

// stdlib includes
#include <thread>
#include <iostream>
#include <mutex>
#include <stack>
#include <map>
#include <unordered_map>

class JobQueue;

class Thread {
  public:
    Thread(const bool &aliveFlag, int index);
    ~Thread();

    /* Thread Interface */
    void enableQueue(JobQueue *queue, unsigned int priority = 1);
    void enableExclusiveQueue(JobQueue *queue);
    
    void disableAllQueues();
    bool hasExclusiveQueue(); // if the thread has been assigned to a single queue exclusively (for now that's the only simplistic mechanism)

    void finish();

    int getIndex() const {
        return threadIndex_;
    }

  private:
    /* Thread State */
    std::thread &thr_ = std::thread(&Thread::workerThreadRun, this);
    
    int threadIndex_;

    // flag that determines whether thread keeps running (should come from ThreadPool)
    bool keepRunning_ = true;
    
    // quick bool for querying whether a thread has an exclusive thread assigned already
      // TODO - evaluate the exclusive queue assignment mechanism, make sure it works 
    bool hasExclusiveQueue_ = false;

    // synchronization
    std::mutex threadDataLock_;
    std::condition_variable threadCondVar_; 

    std::vector<JobQueue*> activeQueues_;
    std::map<unsigned int, unsigned int, std::greater<unsigned int>> activeQueuePriorities_; // gonnna use indices instead of string

    /* Thread Internal Functions */

    // functions for locking/unlocking the thread data mutex
      // probably not useful
    bool acquireDataLockNonblock();
    void acquireDataLockBlock();
    void releaseDataLock();

    /* Worker Thread Functions */

    void workerThreadRun(); // maybe just a simple startup function for now
};
