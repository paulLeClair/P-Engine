#pragma once

#include "../../thread_pool/ThreadPool.hpp"

// stdlib includes
#include <string>
#include <mutex>
#include <vector>

#include <deque>
#include <functional>
#include <future>

template<typename F, typename... Args> using JobReturnType = typename std::result_of<F(Args...)>::type; 

class JobQueue {
  public: 

    JobQueue(const std::string &name, ThreadPool &threadPool); // i guess queue priority will be external to the job queue itself
    JobQueue(ThreadPool &threadPool);
    JobQueue(JobQueue&) = default;
    ~JobQueue();
    
    // huge credits to the amazing c++11 threadpool by Atanas Rusev and Ferai Ali;
        // their threadpool design allows for multiple input types (lambda, function, functor), which seems really useful
    // it also does this very concisely using mostly stuff from <functional> + <future> based on my initial glance, so it could be a very
    // good option

    template <typename F, typename... Args>
    auto scheduleTask(F&& f, Args&&... args /*, Priority priority = Priority::normal*/) 
        -> std::future<JobReturnType<F, Args...>> // using a trailing return type since the return type depends on the template arguments
    {
        // first we package up the job into an executable std::packaged_task(), hiding the actual arguments/return type so everything can be treated
        // as a void function with no arguments 
        auto job = std::make_shared<std::packaged_task<JobReturnType<F, Args...>()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        pushPackagedJob([job] { (*job)(); }); // to convert the std::packaged_task<JobReturnType<F, Args>> to std::function<void()> we just define a small lambda 
        return job->get_future(); // then we just return a future corresponding to the job return type from the packaged_task interface
    }

    std::function<void()> popTask() {
        std::unique_lock<std::mutex> lock(lock_);

        auto job = queue_.front();
        queue_.pop_front();
    }

    // queue activation/deactivation should probably be controlled externally
      // i need to wrap things up in namespaces to illustrate relationships between things better (this JobQueue object is a ThreadPool construct used to
      // submit work to the ThreadPool by external components)
    
    void clear(); 

    // should check if the queue is empty (using internal synch)
    bool isEmpty();

    // simple check whether the queue's lock is free
    std::function<void()> checkForWork();

    // flush queue 
    void flush();

    void submit(); 

  private:
    /* Queue State */
    
    // name of queue
    std::string name_;

    // associated ThreadPool
    ThreadPool &pool_;

    // flag for activity
    bool active_ = false;

    // SYNCHRONIZATION
    // overall queue lock, used when modifying the queue's data structure(s), eg when adding a job to the queue
    std::mutex lock_;

    // condition variable for checking the queue status
    std::condition_variable statusCV_;
    
    // actual queue 
    std::deque<std::function<void()>> queue_;

    // any other synchronization variables?    

    // internal functions
    void pushPackagedJob(std::function<void()> job/*, Priority priority = Priority::normal*/); // removing priority from here for now 
    
    // these are prob not needed now
    void acquireLockBlock();
    bool acquireLockNonblock();
    void releaseLock();

};
