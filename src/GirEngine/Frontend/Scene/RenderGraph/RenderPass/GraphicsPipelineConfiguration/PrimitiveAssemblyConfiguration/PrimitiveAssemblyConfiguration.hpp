//
// Created by paull on 2023-10-26.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    /**
     * All of this is ripped right out of the gir struct (for now(?))
     */
    struct PrimitiveAssemblyConfiguration {
        /**
         * These are possible options for graphics primitives that are
         * supported by the graphics pipeline. Note that in APIs like Vulkan,
         * this may require creation of multiple graphics pipelines since each
         * graphics pipeline can only have 1 primitive topology selected.
         * \n\n
         * However for IR purposes we can just store the various options the user wants here.
         */
        enum class PrimitiveTopology {
            POINT_LIST = 0,
            LINE_LIST = 1,
            LINE_STRIP = 2,
            TRIANGLE_LIST = 3,
            TRIANGLE_STRIP = 4,
            TRIANGLE_FAN = 5,
            LINE_LIST_WITH_ADJACENCY = 6,
            LINE_STRIP_WITH_ADJACENCY = 7,
            TRIANGLE_LIST_WITH_ADJACENCY = 8,
            TRIANGLE_STRIP_WITH_ADJACENCY = 9,
            PATCH_LIST = 10
        };

        /**
         * This setting "controls whether a special vertex index value is treated as restarting the
         * assembly of primitives. This enable only applies to indexed draws, Primitive restart is not allowed for
         * “list” topologies, unless one of the features for such topologies is enabled."
         * \n\n
         * That was taken from the Vulkan spec, this is TODO until I learn more about it
         */
        bool enablePrimitiveRestartForIndexedDraws = false;

        /**
         * This is the collection of primitive topologies that the user wants the graphics pipeline to support.
         */
        std::unordered_set<PrimitiveTopology> enabledPrimitiveTopologies = {};
    };

} // pipeline
