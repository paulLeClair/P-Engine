//
// Created by paull on 2023-10-26.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    struct TessellationConfiguration {
        /**
         * Controls whether tessellation is enabled at all for this pipeline.
         */
        bool enableTessellation = false;

        /**
         * Controls the number of control points per patch if tessellation is enabled
         */
        uint32_t numberOfPatchControlPoints = 0;

    };

} // pipeline
