#pragma once

#include <vector>

#include "../ShaderModuleIR.hpp"

namespace pEngine::girEngine::gir {
/**
 * This is a fairly big one - we can probably shift a lot of the reflection logic
 * into these modules, since they're constructed at bake time and that's when
 * we'll be figuring out and storing information that gets reflected.
 *
 * As a side note: I'm thinking reflection will probably happen in the
 *
 * I'm not sure whether to split these up into an interface & subclasses, but since
 * we'll have to reflect based on the shader language, that seems likely.
 *
 * We can save that for the implementation pass though
*/
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
        struct CreationInput : public ShaderModuleIR::CreationInput {
            std::vector<unsigned int> spirVCode;
        };

        explicit SpirVShaderModuleIR(const CreationInput &creationInput)
                : ShaderModuleIR(creationInput),
                  spirVCode(creationInput.spirVCode) {
            // not sure if this sanity check is worth it but why not
            if (getTypeOfShaderIR() != ShaderModuleIR::IntermediateRepresentation::SPIR_V) {
                throw std::runtime_error(
                        "Error in SpirVShaderModuleIR(): ShaderModuleIR's intermediate representation "
                        "setting is not SPIR_V!");
            }
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