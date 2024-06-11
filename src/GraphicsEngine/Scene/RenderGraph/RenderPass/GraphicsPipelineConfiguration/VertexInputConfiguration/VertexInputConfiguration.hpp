//
// Created by paull on 2023-10-26.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    struct VertexInputConfiguration {
        /**
         * If this is false, then none of this is used and all the information is reflected
         * by the backend. Otherwise it will use the data in this structure instead.
         *
         */
        bool enableManualSpecification = false;

        // TODO - represent vertex bindings as high-level IR;
        //  for now they will be assumed to be reflected from shaders by the backend.
    };

} // pipeline
