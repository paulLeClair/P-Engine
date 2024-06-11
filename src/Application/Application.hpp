//
// Created by paull on 2023-11-11.
//

#pragma once

#include <string>
#include <memory>
#include "../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../EngineCore/EngineCore.hpp"
#include "EngineMode/EngineMode.hpp"

namespace pEngine::app {

    /**
     * The idea here is to group up the engine modes into an application;
     * this way we can just have the user specify the engine modes (and other info about how to transition
     * between them) and then you just have to create one of these in your main() to run the engine.
     *
     * Seems like an easy way to do it, and probably pretty versatile I hope?
     *
     * Note: a lot of required functionality for setting up complicated FSM type things with engine modes is still
     * very much TODO but coming soon
     *
     * One issue I just realized might become ugly: what's the best way to pass in the higher-level engine core
     * stuff to the engine modes? If the user builds them ahead of time, at some point we're going to have
     * to pass in the EngineCore. Which means either uninitialized member variables that have to be set (ew),
     * or we do something that has the application build the engine modes after it builds the core itself (which
     * would probably be done by making the application something you subclass) (also ew).
     *
     * This is a tricky little knot to figure out... Honestly I think between passing in the engine core as an arg to
     * the engine mode methods on one hand, and just having each engine mode have a handle to an engine core on the other,
     * the second option is easiest.
     *
     * That way you can also potentially skip building an Application and just run a single engine mode if you want.
     *
     * I'm going to be trying to do this with templates
     */
    template<typename GirGeneratorType, typename BackendType>
    class Application {
        static_assert(std::is_base_of<gir::generator::GirGenerator, GirGeneratorType>::value,
                      "GirGeneratorType is not a subclass of gir::generator::GirGenerator.");
        static_assert(std::is_base_of<backend::GraphicsBackend, BackendType>::value,
                      "BackendType is not a subclass of backend::GraphicsBackend");

    public:
        struct CreationInput {
            std::string name;
            util::UniqueIdentifier uid;

            /**
             *
             */
            std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> engineCore;

            /**
             * This should be the engine modes the user wants to use in order of execution
             */
            std::vector<std::shared_ptr<mode::EngineMode<GirGeneratorType, BackendType>>> engineModes;
        };

        explicit Application(const CreationInput &creationInput) {

        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const UniqueIdentifier &getUid() const {
            return uid;
        }

        [[nodiscard]] const std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> &getEngineCore() const {
            return engineCore;
        }

        void start() {
            // TODO - expand this to support more complicated specification of different mode transitions etc;
            // for now, it will just take the first engine mode in the list and enter it;
            // since we're assuming that engine modes will handle flowing between each other internally
            // this should be okay for this initial pass over it
            if (!engineModes.empty()) {
                // TODO - probably verify that the engine mode's registered engine core matches up with the application's
                engineModes[0]->enter();
            }
            // TODO - maybe some sort of logging that the application has finished running
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        // the application will maintain probably just one engine core;
        std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> engineCore;

        /**
         * This should be the engine modes the user wants to use in order of execution
         */
        std::vector<std::shared_ptr<mode::EngineMode<GirGeneratorType, BackendType>>> engineModes;

    };

} // app
