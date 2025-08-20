//
// Created by paull on 2023-11-15.
//

#pragma once

#include <vector>

// TODO -> make a central GIR header that declares all gir types without defining them
#include "../GraphicsIR/RenderGraphIR/RenderGraphIR.hpp"

namespace pEngine::girEngine::frontend {
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
        std::vector<gir::BufferIR> buffers = {};
        std::vector<gir::ImageIR> images = {};
        std::vector<gir::ShaderConstantIR> shaderConstants = {};

        // renderables/geometry
        std::vector<gir::model::ModelIR> models = {};
        std::vector<gir::DrawAttachmentIR> drawAttachments = {};
        // these should be unused for single-anim-model demo

        // shader modules
        std::vector<gir::SpirVShaderModuleIR> shaders = {};

        // render graphs
        std::vector<gir::RenderGraphIR> renderGraphs = {};
    };

    struct Frontend {
        virtual ~Frontend() = default;

        [[nodiscard]] virtual BakeOutput bakeToGirs() = 0;
    };
} // generator
