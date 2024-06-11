//
// Created by paull on 2022-08-05.
//

#include <gtest/gtest.h>

#include "../../../../../src/Application/ThreadPool/PThreadPool/Job/Job.hpp"

using namespace PEngine;

class JobTest : public ::testing::Test {
protected:
    std::shared_ptr<Job> testJob = nullptr;
};

TEST_F(JobTest, CreateBasicValidJob) {
    Job::Priority basicValidJobPriority = Job::Priority::HIGH;
    auto basicValidJobFunction = [&](const unsigned int &) {
        // something valid
    };

    Job::CreationInput basicJobCreateInfo = {
            basicValidJobPriority,
            basicValidJobFunction
    };

    ASSERT_NO_THROW(testJob = std::make_shared<Job>(basicJobCreateInfo));
    ASSERT_TRUE(testJob->getPriority() == Job::Priority::HIGH);
}

TEST_F(JobTest, ExecuteJobSuccessfully) {
    int sum = 0;

    Job::CreationInput successfulJobCreateInfo = {
            Job::Priority::LOW,
            [&](const unsigned int &) {
                sum += 5;
            }
    };
    ASSERT_NO_THROW(testJob = std::make_shared<Job>(successfulJobCreateInfo));

    auto result = (*testJob)();
    ASSERT_EQ(result, Job::ExecuteJobResult::SUCCESS);
    ASSERT_EQ(sum, 5);
}

TEST_F(JobTest, ExecuteJobUnsuccessfully) {
    // TODO - flesh this out to cover more ways that a job might fail besides throwing a runtime exception (although im not sure what else there is rn)
    Job::CreationInput unsuccessfulJobCreateInfo = {
            Job::Priority::LOW,
            [&](const unsigned int &) {
                throw std::runtime_error("The job failed to execute properly and the program needs to stop!");
            }
    };
    ASSERT_NO_THROW(testJob = std::make_shared<Job>(unsuccessfulJobCreateInfo));

    auto result = (*testJob)();
    ASSERT_EQ(result, Job::ExecuteJobResult::FAILURE);
}