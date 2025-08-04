//
// Created by paull on 2022-06-18.
//

#pragma once

#include <fstream>
#include <filesystem>

#include "../../GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp"
#include "../../GraphicsIR/ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIR.hpp"

namespace pEngine::girEngine::gir {
    class GraphicsIntermediateRepresentation;
}

namespace pEngine::girEngine::scene {
    /**
     * I'm not sure how the final design will look, but I think the
     * scene-facing shader module will just read in the shader file data
     * itself, and then the backend will actually process the shader.
     *
     */
    class ShaderModule {
    public:
        enum class ShaderUsage {
            VERTEX_SHADER = 1,
            FRAGMENT_SHADER = 2,
            TESSELLATION_EVALUATION_SHADER = 4,
            TESSELLATION_CONTROL_SHADER = 8,
            GEOMETRY_SHADER = 16,
            MESH_TASK_SHADER = 32,
            MESH_SHADER = 64
            // TODO - add more shader usage types
        };

        enum class ShaderLanguage {
            HLSL,
            GLSL,
            MSL
        };

        struct CreationInput {
            std::string name;
            UniqueIdentifier uid;

            std::string shaderFileNameWithoutFileExtension;
            std::string shaderEntryPointName;
            ShaderUsage enabledShaderUsages;
            ShaderLanguage shaderFileType;
        };

        explicit ShaderModule(const CreationInput &createInfo) : name(createInfo.name),
                                                                 uid(createInfo.uid),
                                                                 shaderFileName(
                                                                     createInfo.shaderFileNameWithoutFileExtension),
                                                                 usage(createInfo.enabledShaderUsages),
                                                                 shaderFileType(createInfo.shaderFileType),
                                                                 shaderEntryPointName(createInfo.shaderEntryPointName) {
        }

        ShaderModule() = default;

        [[nodiscard]] const UniqueIdentifier &getUid() const {
            return uid;
        }

        [[nodiscard]] const std::string &getName() const {
            return name;
        }

        [[nodiscard]] const std::string &getShaderFileName() const {
            return shaderFileName;
        }

        [[nodiscard]] ShaderUsage getUsage() const {
            return usage;
        }

        [[nodiscard]] ShaderLanguage getShaderFileType() const {
            return shaderFileType;
        }

        gir::SpirVShaderModuleIR bakeToGIR() const {
            // TODO - extend this when there are more than just spir V shader modules
            return gir::SpirVShaderModuleIR(gir::SpirVShaderModuleIR::CreationInput{
                name,
                uid,
                gir::GIRSubtype::SHADER_MODULE,
                shaderFileName,
                gir::ShaderModuleIR::IntermediateRepresentation::SPIR_V,
                getGirShaderModuleUsage(usage),
                shaderEntryPointName,
                getSpirVCodeFromShaderModuleFile(
                    shaderFileType, shaderFileName, usage)
            });
        }

    private:
        util::UniqueIdentifier uid;
        std::string name;
        std::string shaderFileName;
        std::string shaderEntryPointName;
        ShaderUsage usage;
        ShaderLanguage shaderFileType;

        static gir::SpirVShaderModuleIR::ShaderUsage
        getGirShaderModuleUsage(ShaderUsage sceneShaderUsages) {
            switch (sceneShaderUsages) {
                case (ShaderUsage::VERTEX_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::VERTEX_SHADER;
                }
                case (ShaderUsage::FRAGMENT_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::FRAGMENT_SHADER;
                }
                case (ShaderUsage::TESSELLATION_EVALUATION_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::TESSELLATION_EVALUATION_SHADER;
                }
                case (ShaderUsage::TESSELLATION_CONTROL_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::TESSELLATION_CONTROL_SHADER;
                }
                case (ShaderUsage::GEOMETRY_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::GEOMETRY_SHADER;
                }
                case (ShaderUsage::MESH_TASK_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::MESH_TASK_SHADER;
                }
                case (ShaderUsage::MESH_SHADER): {
                    return gir::SpirVShaderModuleIR::ShaderUsage::MESH_SHADER;
                }
                default: {
                    throw std::runtime_error(
                        "Error in PShaderModule::getGirShaderModuleUsage() - unrecognized scene shader usage");
                }
            }
        }

        static std::vector<unsigned int>
        getSpirVCodeFromShaderModuleFile(ShaderLanguage shaderLanguage,
                                         const std::string &fileName,
                                         ShaderUsage shaderUsage) {
            std::vector<unsigned int> spirVCode = {};

            // NOTE - this forces the C++ standard to be 17 or greater
            std::string shaderModulePath = getShaderModuleBinaryPath(
                fileName,
                shaderLanguage,
                shaderUsage);

            std::ifstream file(shaderModulePath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                throw std::runtime_error("Unable to open shader file " + shaderModulePath);
            }

            auto fileSize = static_cast<uint32_t>(file.tellg());
            unsigned int packedFileSize = fileSize / sizeof(uint32_t);

            spirVCode.resize(packedFileSize);
            file.seekg(0);
            file.read((char *) spirVCode.data(), fileSize);
            file.close();

            return spirVCode;
        }

        static std::string
        getShaderModuleBinaryPath(
            const std::string &fileName,
            ShaderLanguage shaderLanguage,
            ShaderUsage shaderUsage) {
            static const char *const SHADER_BINARIES_PROJECT_RELATIVE_PATH = R"(src\shaders\bin\)";
            static const char *const WIN32_INSTALL_HARD_DRIVE_PATH = "C:\\";

            std::string shaderModuleName = fileName + getShaderModuleFileExtension(shaderLanguage,
                                               shaderUsage);
            // NOTE - this forces the C++ standard to be 17 or greater
            std::filesystem::path currentPath = std::filesystem::current_path().relative_path();
            std::filesystem::path shaderModulePath;
#ifdef _WIN32
            shaderModulePath = std::filesystem::path(WIN32_INSTALL_HARD_DRIVE_PATH);
#endif
            for (const auto &currentRelativePathElementItr: currentPath) {
                if (currentRelativePathElementItr.string() == "build") {
                    break;
                }

                shaderModulePath += currentRelativePathElementItr;
                shaderModulePath += std::filesystem::path("\\");
            }

            std::string shaderSpirVModuleName
                    = shaderModuleName.substr(0, shaderModuleName.find_first_of('.')) + ".spv";
            shaderModulePath += std::filesystem::path(SHADER_BINARIES_PROJECT_RELATIVE_PATH + shaderSpirVModuleName);
            return shaderModulePath.string();
        }

        static std::string getShaderModuleFileExtension(
            ShaderLanguage type,
            ShaderUsage shaderUsage) {
            switch (type) {
                case (ShaderLanguage::GLSL): {
                    switch (shaderUsage) {
                        case (ShaderUsage::VERTEX_SHADER): {
                            return ".vert";
                        }
                        case (ShaderUsage::FRAGMENT_SHADER): {
                            return ".frag";
                        }
                        case (ShaderUsage::GEOMETRY_SHADER): {
                            return ".geom";
                        }
                        case (ShaderUsage::TESSELLATION_CONTROL_SHADER): {
                            return ".tesc";
                        }
                        case (ShaderUsage::TESSELLATION_EVALUATION_SHADER): {
                            return ".tese";
                        }
                        default:
                            return ".glsl";
                    }
                }
                case (ShaderLanguage::HLSL):
                    return ".hlsl";
                case (ShaderLanguage::MSL):
                    return ".msl";
                default: {
                    throw std::runtime_error(
                        "Error in PShaderModule::getShaderModuleFileExtension() - unrecognized shader file type");
                }
            }
        }
    };
} // scene
