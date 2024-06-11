#pragma once

#include <utility>

#include "Backend/Backend.hpp"
#include "Scene/Scene.hpp"

/**
 * Returning to this as part of the backend rewrite;
 *
 * I think it probably does make sense to just keep this and have it kind of be the anchor point
 * where the rest of the application "talks" to the graphics side of application.
 *
 * The difficulty might be coming up with an interface here that is general enough.
 * But hopefully I can come up with something ultra simple that works for now, and then
 * we can return to it later maybe if it's not completely adequate or whatever.
 *
 * That said, if it's just handing off the handles to things that handle various things,
 * and then maybe provide a couple overall methods for like "draw next frame" or
 * "bake" or whatever; that might be good enough. We'll see
 *
 */

namespace pEngine::girEngine {

    template<typename GirGeneratorType, typename BackendType>
    class GraphicsEngine {
        /**
         * These static asserts should make sure that the GirGeneratorType and GraphicsBackendTypes given are
         * subclasses of girEngine::scene::Scene and girEngine::backend::Backend.
         *
         * Not sure if this is the cleanest or best way to do this, but it should work for now.
         */
        static_assert(std::is_base_of<scene::Scene, GirGeneratorType>::value,
                      "GirGeneratorType is not a subclass of scene::Scene.");
        static_assert(std::is_base_of<backend::GraphicsBackend, BackendType>::value,
                      "BackendType is not a subclass of backend::GraphicsBackend");
    public:
        struct CreationInput {
            CreationInput(std::string label, const std::shared_ptr<GirGeneratorType> &scene,
                          const std::shared_ptr<BackendType> &backend) : label(std::move(label)), scene(scene),
                                                                         backend(backend) {}

            std::string label;

            std::shared_ptr<GirGeneratorType> scene;
            std::shared_ptr<BackendType> backend;
        };

        explicit GraphicsEngine(const CreationInput &creationInput) : label(creationInput.label),
                                                                      scene(creationInput.scene),
                                                                      backend(creationInput.backend) {

        }

        ~GraphicsEngine() = default;

        [[nodiscard]] const std::string &getLabel() const {
            return label;
        }

        [[nodiscard]] const std::shared_ptr<GirGeneratorType> &getScene() const {
            return scene;
        }

        [[nodiscard]] const std::shared_ptr<BackendType> &getBackend() const {
            return backend;
        }

    private:
        std::string label;

        std::shared_ptr<GirGeneratorType> scene;
        std::shared_ptr<BackendType> backend;
    };

}