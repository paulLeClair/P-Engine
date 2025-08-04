//
// Created by paull on 2022-08-30.
//

#pragma once

#include <stdexcept>

#include "../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../EngineCore/EngineCore.hpp"

namespace pEngine::app::mode {

    /**
     * Okay so now we're finally rewriting this engine mode thing.
     *
     * The idea here is that we want to use these engine mode classes to be
     * programmable "states" to be in for the overall application provided by the engine
     *
     * That kinda makes sense I think, and then it allows you to break up your application into chunks
     * as well as define how different chunks can transition between each other.
     *
     * For games, you could do everything from a launcher to the main menu to whatever other general "modes" you want your
     * application to be in.
     *
     * At least that's the hope lol, I'll have to give it a shot and see how it develops...
     *
     * For now, I think we can maybe keep it fairly simple:
     * - each engine mode gets a pointer at creation time to the next engine mode that should be executed
     * - the engine mode has access to the engine core (which should hold on to the thread pool most likely?) and the graphics engine itself
     * - the engine mode should define a few methods, something like "setup() -> bake() -> run() -> finish()" sort of thing, which define how the engine mode proceeds
     *
     * Then all the juicy shit just happens inside these methods that have consistent usages across all engine modes.
     *
     *
     * Another important thing to remember is that we want to overall be supporting applications with an
     * "update() -> render()" loop; I think for versatility's sake it makes sense to provide
     * general-purpose implementations of "bake()" and "finish()"; "setup()" is probably best left to be pure virtual
     * so that the user has to define what happens there, and "run()" is a whole other beast.
     *
     * This might be where we want to make a class which provides a customizable infrastructure for writing
     * the "update()->render()" loops that the user's application wants. That seems like a good thing to do,
     * since we want it to be robust but it's also optional to use them if we break them out. The user should
     * technically be free to define the "update()->render()" loop however they want.
     *
     */
    template<typename GirGeneratorType, typename BackendType>
    class EngineMode {
        /**
         * These static asserts should make sure that the GirGeneratorType and GraphicsBackendTypes given are
         * subclasses of girEngine::scene::Scene and girEngine::backend::Backend.
         *
         * Not sure if this is the cleanest or best way to do this, but it should work for now.
         *
         */
        static_assert(std::is_base_of<gir::generator::GirGenerator, GirGeneratorType>::value,
                      "GirGeneratorType is not a subclass of gir::generator::GirGenerator.");
        static_assert(std::is_base_of<backend::GraphicsBackend, BackendType>::value,
                      "BackendType is not a subclass of backend::GraphicsBackend");

    public:
        struct CreationInput {
            std::string name;

            std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> engineCore;

            // TODO - extend this to allow for the possibility of branching to multiple different engine modes
            // (after we get singular engine modes working)
            std::shared_ptr<EngineMode<GirGeneratorType, BackendType>> nextEngineMode = nullptr;
        };

        explicit EngineMode(const CreationInput &creationInput) : name(creationInput.name),
                                                                  uid(util::UniqueIdentifier()),
                                                                  engineCore(creationInput.engineCore),
                                                                  nextEngineMode(creationInput.nextEngineMode) {

        }

        virtual ~EngineMode() = default;

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const util::UniqueIdentifier &getUid() const {
            return uid;
        }

        [[nodiscard]] const std::shared_ptr<EngineMode<GirGeneratorType, BackendType>> &getNextEngineMode() const {
            return nextEngineMode;
        }

        /**
         * All engine modes will maintain a handle to a working engine core;
         * from here subclass engine modes can obtain particular pieces as they see fit
         * to avoid constantly calling "getEngineCore()" and dereferencing it
         * @return
         */
        [[nodiscard]] const std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> &getEngineCore() const {
            return engineCore;
        }

        virtual void begin() = 0;


    private:
        std::string name;
        util::UniqueIdentifier uid;

        std::shared_ptr<core::EngineCore<GirGeneratorType, BackendType>> engineCore;

        std::shared_ptr<EngineMode<GirGeneratorType, BackendType>> nextEngineMode = nullptr;
    };

}