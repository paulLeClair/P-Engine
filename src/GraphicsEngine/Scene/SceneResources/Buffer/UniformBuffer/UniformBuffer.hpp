#pragma once

#include "../Buffer.hpp"

namespace pEngine::girEngine::scene {
    class UniformBuffer : public Buffer {
    public:
        struct CreationInput : public Buffer::CreationInput {
            // whatever else is required for a uniform buffer
        };

        explicit UniformBuffer(const CreationInput &creationInput) : Buffer(creationInput) {
        }
    };
}
