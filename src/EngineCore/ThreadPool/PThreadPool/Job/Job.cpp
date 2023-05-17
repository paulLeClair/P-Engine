//
// Created by paull on 2022-07-30.
//

#include "Job.hpp"

#include "../Thread/Thread.hpp"

namespace PEngine {


    Job::Job(const Job::CreationInput &createInfo) : priority(createInfo.priority),
                                                     jobFunction(createInfo.jobFunction) {
    }

    Job::ExecuteJobResult Job::executeJob(const Thread &thread) {
        try {
            jobFunction(thread.getIndex());
            return ExecuteJobResult::SUCCESS;
        } catch (const std::exception &exception) {
            return ExecuteJobResult::FAILURE;
        }
    }

    Job::Priority Job::getPriority() const {
        return priority;
    }

    [[maybe_unused]] const std::function<void(const unsigned int &)> &Job::getJobFunction() const {
        return jobFunction;
    }

    Job::ExecuteJobResult Job::executeJobOnCallingThread() {
        try {
            jobFunction(0);
            return Job::ExecuteJobResult::SUCCESS;
        }
        catch (std::exception &exception) {
            return Job::ExecuteJobResult::FAILURE;
        }
    }

}// namespace PEngine