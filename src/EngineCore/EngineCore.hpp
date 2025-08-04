//
// Created by paull on 2022-05-30.
//

#pragma once

#include <memory>
#include <utility>

#include "../lib/bs_thread_pool/BS_thread_pool.hpp"

#include "../GraphicsEngine/GraphicsEngine.hpp"

using namespace pEngine::girEngine;

namespace pEngine::core {
    template<typename GirGeneratorType, typename BackendType>
    class EngineCore {
        /**
         * These static asserts should make sure that the GirGeneratorType and GraphicsBackendTypes given are
         * subclasses of girEngine::gir::generator::GirGenerator and girEngine::backend::Backend.
         *
         * Not sure if this is the cleanest or best way to do this, but it should work for now.
         */
        static_assert(std::is_base_of<gir::generator::GirGenerator, GirGeneratorType>::value,
                      "GirGeneratorType is not a subclass of gir::generator::GirGenerator.");
        static_assert(std::is_base_of<backend::GraphicsBackend, BackendType>::value,
                      "BackendType is not a subclass of backend::GraphicsBackend");

    public:
        struct CreationInput {
            std::string name;

            /**
             * This is the number of worker threads that will be given to the thread pool;
             * Leave it as 0 to have the value be determined automatically.
             */
            unsigned int numberOfThreadPoolWorkerThreads = 0;

            std::shared_ptr<GirGeneratorType> coreScene;

            std::shared_ptr<BackendType> coreBackend;
        };

        explicit EngineCore(const CreationInput &creationInput)
            : name(creationInput.name),
              uid(util::UniqueIdentifier()) {
            // use specified number of worker threads if a value was given
            if (creationInput.numberOfThreadPoolWorkerThreads) {
                threadPool = std::make_unique<BS::thread_pool>(creationInput.numberOfThreadPoolWorkerThreads);
            }
            threadPool = std::make_unique<BS::thread_pool>();

            typename pEngine::girEngine::GraphicsEngine<GirGeneratorType, BackendType>::CreationInput
                    graphicsEngineCreationInput(
                        "label",
                        creationInput.coreScene,
                        creationInput.coreBackend
                    );
            graphicsEngine = std::make_unique<pEngine::girEngine::GraphicsEngine<GirGeneratorType, BackendType> >(
                graphicsEngineCreationInput);
        }

        ~EngineCore() = default;

        [[nodiscard]] const BS::thread_pool &getThreadPool() const {
            return *threadPool;
        }

        [[nodiscard]] const girEngine::GraphicsEngine<GirGeneratorType, BackendType> &getGraphicsEngine() const {
            return *graphicsEngine;
        }

    private:
        std::string name;
        util::UniqueIdentifier uid;

        /**
         * An instance of one of Barack Shoshany's amazing thread pools! Read up :)
         * https://github.com/bshoshany/thread-pool
         */
        std::unique_ptr<BS::thread_pool> threadPool;

        /**
         * Handle to this engine core's graphics engine instance
         */
        std::unique_ptr<girEngine::GraphicsEngine<GirGeneratorType, BackendType> > graphicsEngine;
    };
} // namespace PEngine
