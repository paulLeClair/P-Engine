#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::pipeline {

    /**
     * This should contain the information about vertex attributes (etc) for the pipeline.
     *
     * Honestly, this requires reflection for the most part, and maybe we can leave it optional
     * \n\n
     * On the other hand, we might be able to determine this information
     * from the render pass (each of which has its own graphics pipeline)
     * and the geometry that it's assigned to draw.
     * \n\n
     * Granted it would be cool to just not even have to specify anything about
     * bindings (and maybe provide optional manual specification.
     * \n\n
     * Since I'm not sure yet how this will work, all of this will be TODO for now.
    */
    struct VertexInputStateIR {

        /**
         * If this is false, then none of this is used and all the information is reflected
         * by the backend. Otherwise it will use the data in this structure instead.
         *
         */
        bool enableManualSpecification = false;

        // TODO - represent vertex bindings as high-level IR;
        //  for now they will be assumed to be reflected from shaders by the backend.

    };

}