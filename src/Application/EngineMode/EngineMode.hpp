//
// Created by paull on 2022-08-30.
//

#pragma once

#include <stdexcept>

#include "../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "../../EngineCore/EngineCore.hpp"

namespace pEngine::app::mode {

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
