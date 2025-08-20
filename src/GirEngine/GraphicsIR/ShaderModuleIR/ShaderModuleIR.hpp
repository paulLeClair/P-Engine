//
// Created by paull on 2023-09-30.
//

#pragma once

#include "../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir {
    class ShaderModuleIR : public GraphicsIntermediateRepresentation {
    public:
        enum class IntermediateRepresentation {
            UNKNOWN,
            SPIR_V,
            /**
             * Microsoft's intermediate representation based on LLVM that is compiled from HLSL.
             * There's at least a way to convert from SPIR-V to DXIL, and probably a
             * way to go from DXIL to SPIR-V.
             */
            DXIL
            // TODO - add any other shading language representations here
        };

        enum class ShaderUsage {
            UNKNOWN,
            VERTEX_SHADER,
            GEOMETRY_SHADER,
            TESSELLATION_CONTROL_SHADER,
            TESSELLATION_EVALUATION_SHADER,
            FRAGMENT_SHADER,
            MESH_SHADER,
            MESH_TASK_SHADER
            // TODO - ray tracing stuff
        };

        ShaderModuleIR() = default;

        struct CreationInput : GraphicsIntermediateRepresentation::CreationInput {
            std::string shaderModuleFilename;
            IntermediateRepresentation girType = IntermediateRepresentation::UNKNOWN;
            ShaderUsage usage = ShaderUsage::UNKNOWN;
            std::string shaderModuleEntryPointName;
        };

        explicit ShaderModuleIR(const CreationInput &creationInput)
            : GraphicsIntermediateRepresentation(creationInput),
              shaderGirType(creationInput.girType),
              usage(creationInput.usage),
              shaderModuleFile(creationInput.shaderModuleFilename),
              shaderModuleEntryPointName(creationInput.shaderModuleEntryPointName) {
        }

        [[nodiscard]] IntermediateRepresentation getTypeOfShaderIR() const {
            return shaderGirType;
        }

        [[nodiscard]] ShaderUsage getUsage() const {
            return usage;
        }

        [[nodiscard]] const std::string &getFilename() const {
            return shaderModuleFile;
        }

        [[nodiscard]] const std::string &getEntryPointName() const {
            return shaderModuleEntryPointName;
        }

    private:
        IntermediateRepresentation shaderGirType = IntermediateRepresentation::UNKNOWN;
        ShaderUsage usage = ShaderUsage::UNKNOWN;

        /**
         * This should be the name of the spir-v module that the shader module corresponds to, without the file extension.
         */
        std::string shaderModuleFile;

        std::string shaderModuleEntryPointName;
    };
} // gir
