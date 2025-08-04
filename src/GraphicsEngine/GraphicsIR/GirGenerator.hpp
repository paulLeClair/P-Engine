//
// Created by paull on 2023-11-15.
//

#pragma once

#include <vector>
#include <memory>

#include "GraphicsIntermediateRepresentation.hpp"
#include "CameraGIR/CameraGIR.hpp"
#include "light/PointLightIR/PointLightIR.hpp"
#include "RenderGraphIR/RenderGraphIR.hpp"


namespace pEngine::girEngine::gir::generator {
    struct GirGenerator {
        virtual ~GirGenerator() = default;

        /**
         * This will define the currently-expected output format for the various GIRS that
         * a GIRGenerator implementation will produce, separated out into an SoA format;
         * once we re-implement the scene's RenderGraph representation, I'll probably end up
         * modifying a lot of this.\n\n
         *
         * For the simple linear bake process, it should be fine though.\n\n
         *
         * It will grow as more things are wired into the engine but for now I'm going to keep it trim
         * and only include things that are directly used for the current single-animated-model demo.
         */
        struct BakeOutput {
            // buffer/image resources
            std::vector<BufferIR> buffers = {};
            std::vector<ImageIR> images = {};
            std::vector<ShaderConstantIR> shaderConstants = {};

            // renderables/geometry
            std::vector<model::ModelIR> models = {};
            std::vector<DrawAttachmentIR> drawAttachments = {};
            // these should be unused for single-anim-model demo

            // misc (THESE ARE PROBABLY DEPRECATED)
            std::vector<camera::CameraGIR> cameras = {};
            std::vector<light::PointLightIR> pointLights = {};

            // shader modules
            std::vector<SpirVShaderModuleIR> shaders = {};

            // render graphs
            std::vector<RenderGraphIR> renderGraphs = {};
        };

        [[nodiscard]] virtual BakeOutput bakeToGirs() = 0;
    };
} // generator
