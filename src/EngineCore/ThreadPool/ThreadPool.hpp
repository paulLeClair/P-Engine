//
// Created by paull on 2022-07-23.
//

#pragma once

/**
 * TODO - break off PThreadPool methods into this abstract class
 */
class ThreadPool {
public:
  ~ThreadPool() = default;

  virtual unsigned int getNumberOfWorkerThreads() = 0;


private:
};
