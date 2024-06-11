//
// Created by paull on 2022-07-30.
//

#pragma once

#include <functional>
#include <utility>

namespace pEngine::threadPool {

    // TODO - rewrite/probably rip all this out of here as part of thread pool rewrite... or repurpose if possible

    template<typename CallableType, typename... CallArgs>
    using JobReturnType = typename std::result_of<CallableType(CallArgs...)>::type;

    class Thread;

/**
 * This should correspond to a single unit of work that will be executed *to completion* by a single thread.
 *
 * As with all the thread pool components, this follows the awesome example set by Atanas Rusev and Ferai Ali!
 * Will have to make sure I put a link somewhere in the documentation.
 *
 * Internally, jobs will be represented as std::functions, so that we can supply them in a few different ways,
 * which will be flexible enough for our purposes (I believe)
 */
    class Job {
    public:
        enum class Priority : size_t {
            LOW = 0u,
            MEDIUM = 1u,
            HIGH = 2u,
            CRITICAL = 3u,
            MAX_ENUM = 4u
        };

        static const Priority DEFAULT_PRIORITY = Priority::MEDIUM;// TODO - make this configurable

        struct CreationInput {
            Priority priority = DEFAULT_PRIORITY;
            std::function<void(const unsigned int &)> jobFunction;
        };

        explicit Job(const CreationInput &createInfo);

        Job()
                : priority(DEFAULT_PRIORITY),
                  jobFunction([&](const unsigned int &) {}) {

        }

        Job(const Job &other) = default;

        ~Job() = default;

        enum class ExecuteJobResult {
            SUCCESS,
            FAILURE
        };

        /**
         * This should only be called by a worker thread! Use 'executeJobOnCallingThread' to execute the job function in place.
         */
        ExecuteJobResult executeJob(const Thread &thread);

        ExecuteJobResult executeJobOnCallingThread();

        ExecuteJobResult operator()(const Thread &thread) {
            return executeJob(thread);
        }

        ExecuteJobResult operator()() {
            return executeJobOnCallingThread();
        }

        [[nodiscard]] Priority getPriority() const;

        [[maybe_unused]] [[nodiscard]] const std::function<void(const unsigned int &)> &getJobFunction() const;

    private:
        Priority priority;

        std::function<void(const unsigned int &thread)> jobFunction = [&](const unsigned int &threadIndex) {
            // default: nothing at all; maybe add some sort of logger info here to notify the user when a blank job has been
            // executed?
        };
    };

}// namespace PEngine
