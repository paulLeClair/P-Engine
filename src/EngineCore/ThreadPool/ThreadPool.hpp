//
// Created by paull on 2022-07-23.
//

#pragma once

/**
 * Alright! I'm currently in a car driving from Kelowna to Edmonton so I have no internet, and I think I'll need the
 * internet to move all the thread pool stuff over here
 */
class ThreadPool {
public:
  ~ThreadPool() = default;

  virtual unsigned int getNumberOfWorkerThreads() = 0;


private:
};
