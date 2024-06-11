//
// Created by paull on 2022-07-23.
//

#pragma once

namespace pEngine::threadPool {
    /**
     * Alright! I'm currently in a car driving from Kelowna to Edmonton so I have no internet, and I think I'll need the
     * internet to move all the thread pool stuff over here
     *
     * lol @ the above comment :) no idea why i would need internet for that but maybe i had to pull changes?
     *
     * Back to this over a year later as part of doing the 900th backend rewrite!
     *
     * TODO - write our own version of Barack Shoshany's thread pool (since it's simple and would be informative for me)
     */
    class ThreadPool {
    public:
        struct CreationInput {

        };

        explicit ThreadPool(const CreationInput &creationInput) {

        }

        ~ThreadPool() = default;

        unsigned int getNumberOfWorkerThreads() {

        }


    private:
        // One amazing implementation I've found (which boasts crazy performance and is very lightweight)
        // is Barack Shoshany's Thread Pool: https://github.com/bshoshany/thread-pool

        // it's gonna be a lot easier to just use that for now

        // I think I should try (as an exercise) to duplicate it here and see if I can get the same performance;
        // if I can't then I might just incorporate it as an external dependency since it's so lightweight and
        // this project is c++17 compliant
    };

}
