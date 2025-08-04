#pragma once

#include <vector>

#include "../ShaderModuleIR.hpp"

namespace pEngine::girEngine::gir {

    class SpirVShaderModuleIR : public ShaderModuleIR {
    public:
        /**
         * All information relevant for a spir-v shader module,
         * which most likely is intended for Vulkan backends
         * but potentially also OpenGL if you had the foul
         * inclination to write an OpenGL backend...
         *
         * For Vulkan, it really only needs a pointer
         * to some SPIR-V code (represented as a vector of uint32_t)
         *
         */
        struct CreationInput : ShaderModuleIR::CreationInput {
            std::vector<unsigned int> spirVCode = {};
        };

        SpirVShaderModuleIR() : SpirVShaderModuleIR(CreationInput{}) {
        }

        explicit SpirVShaderModuleIR(const CreationInput &creationInput)
            : ShaderModuleIR(creationInput),
              spirVCode(creationInput.spirVCode) {
        }

        ~SpirVShaderModuleIR() override = default;


        [[nodiscard]] const std::vector<unsigned int> &getSpirVCode() const {
            return spirVCode;
        }

    private:
        /**
         * This is the SPIR-V byte code for the backend, which are packed into 32-bit integers
         */
        std::vector<unsigned int> spirVCode;
    };
}
