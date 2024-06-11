#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::pipeline {

    /**
     * This should contain any configuration for tessellation in the graphics pipeline.
    */
    struct TessellationStateIR {

        /**
         * Controls whether tessellation is enabled at all for this pipeline.
         */
        bool enableTessellation = false;

        /**
         * Controls the number of control points per patch if tessellation is enabled
         */
        uint32_t numberOfPatchControlPoints = 0;
    };

}