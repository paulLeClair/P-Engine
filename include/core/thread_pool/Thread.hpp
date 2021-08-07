#pragma once

// stdlib includes
#include <thread>
#include <iostream>
#include <mutex>
#include <stack>
#include <map>
#include <unordered_map>


// #include "../os_interface/OSInterface.hpp"

/* DESIGN 

- 

*/

class JobQueue;

// i'm thinking it's a good idea to encapsulate the std::thread in a Thread object
class Thread {
  public:
    // enum class ActiveQueueFlag {
    //   active,
    //   exclusive, // the thread has been assigned a specific job queue to watch 
    //   inactive
    // };
        // probably unnecessary rn, plus it would be better in ThreadPool.hpp 

    Thread(const bool &aliveFlag, int index);
    ~Thread();

    /* Thread Interface */
    // void setThreadIndex(int index); // this should only work once
        // no need for this lmao
    
    // function for enabling a queue i guess
    // void enableQueue(std::shared_ptr<JobQueue> queue, std::string name, unsigned int priority); 
    void enableQueue(JobQueue *queue, unsigned int priority = 1);
    // void enableExclusiveQueue(std::shared_ptr<JobQueue> queue, std::string name);
    void enableExclusiveQueue(JobQueue *queue);
    
    // void disableQueue(std::string name);
    void disableAllQueues();
    bool hasExclusiveQueue(); // if the thread has been assigned to a single queue exclusively (for now that's the only simplistic mechanism)

    // finish() - join the thread and wait until it finishes
        //MAR19 - maybe i'll change this so that it causes the thread to stop
    void finish();

    int getIndex() const {
        return threadIndex_;
    }

  private:
    /* Thread State */
    // actual thread
    std::thread &thr_ = std::thread(&Thread::workerThreadRun, this); // thread is created here maybe?
      // store ref 2 thread (should probably look into how you should wrap a std::thread but this seems to work for now)
    
    // just gonna store and set the thread index after creation
    // bool indexHasBeenSet_ = false;
    int threadIndex_;

    // flag that determines whether thread keeps running (should come from ThreadPool)
    // const bool& keepRunning_; // changin this to a thread-specific value
    bool keepRunning_ = true;
    
    // quick bool for querying whether a thread has an exclusive thread assigned already
    bool hasExclusiveQueue_ = false;

    // os pointer? might need it
    // OSInterfacePtr os_;

    // synchronization stuff?
    // probably need a lock for accessing the Thread's data (since the ThreadPool will directly modify the data and the actual std::thread will
    // access the Thread's data, so there has to be synchronization there i beeeelieeeeve)
      // MAR17 - im pretty sure this would be useful for storing thread-specific stuff such as which 
    std::mutex threadDataLock_;
    std::condition_variable threadCondVar_; // might have a use for this: since threads need to access their data but need to allow changes too, the
                                              // controlling threads might need 

    // i guess i'll try having the thread maintain its own maps
    // std::map<unsigned int, std::string, std::greater<unsigned int>> activeQueuePriorities_;
    // std::unordered_map<std::string, std::shared_ptr<JobQueue>> activeQueues_; 
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

    // void workerThreadExit(); // function which should be called when the worker thread determines it should exit
        // not necessary

  protected:

};