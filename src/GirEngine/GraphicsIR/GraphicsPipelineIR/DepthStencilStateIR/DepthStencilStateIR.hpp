#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::pipeline {

    /**
     * This should control depth and stencil testing in the pipeline.
     * As with all graphics pipeline IR classes, these basically just wrap
     * some corresponding Vulkan classes; in the future we can break things up
     * to accommodate other APIs
    */
    struct DepthStencilStateIR {

        /**
         * These are comparison operations that can be used for depth testing.\n\n
         *
         * NOTE/TODO -
         * it may be worthwhile to factor out common things like comparison operations into
         * utilities, but on the other hand it reduces coupling and isn't super
         * painful to just have each class duplicate it a little bit for the
         * sake of context when reading through the code.
         */
        enum class CompareOperation {
            NO_OP,
            NEVER,
            LESS,
            EQUAL,
            LESS_OR_EQUAL,
            GREATER,
            NOT_EQUAL,
            GREATER_OR_EQUAL,
            ALWAYS
        };

        /**
         * Possible stencil operations to be used if stencil testing is enabled
         */
        enum class StencilOperation {
            NO_OP,
            KEEP,
            ZERO,
            REPLACE,
            INCREMENT_AND_CLAMP,
            DECREMENT_AND_CLAMP,
            INVERT,
            INCREMENT_AND_WRAP,
            DECREMENT_AND_WRAP
        };

        /**
         * Modeled after vulkan for now - stores stencil testing parameters
         * for either front-facing or back-facing polygons (see the Vulkan spec
         * section on stencil testing for more information).
         */
        struct StencilTestState {
            StencilOperation failOp = StencilOperation::NO_OP;
            StencilOperation passOp = StencilOperation::NO_OP;
            StencilOperation depthFailOp = StencilOperation::NO_OP;
            CompareOperation compareOp = CompareOperation::NO_OP;

            /**
             * TODO - re-evaluate this implementation when we start the backend bake. \n\n
             *
             * Taken right from Vulkan. \n\n
             *
             * According to the spec:
             * "compareMask selects the bits of the unsigned integer stencil values participating
             * in the stencil test."
             *
             */
            uint32_t compareMask = 0;

            /**
             * TODO - re-evaluate this implementation when we start the backend bake. \n\n
             *
             * Taken right from Vulkan. \n\n
             *
             * According to the spec:
             * "writeMask selects the bits of the unsigned integer stencil values updated by the
             * stencil test in the stencil framebuffer attachment."
             *
             */
            uint32_t writeMask = 0;

            /**
             * TODO - re-evaluate this implementation when we start the backend bake. \n\n
             *
             * Taken right from Vulkan. \n\n
             *
             * According to the spec:
             * "reference is an integer stencil reference value that is used in the unsigned stencil comparison."
             *
             */
            uint32_t stencilReference = 0;
        };

        /**
         * Enables the depth comparison testing operation
         */
        bool enableDepthTesting = false;

        /**
         * Configures whether the graphics pipeline should enable writing
         * to the depth buffer as part of depth testing. \n\n
         *
         * I'm not entirely sure currently when you'd want this off or on, I would
         * think it would pretty much always be on though?
         */
        bool enableDepthWrites = false;

        /**
         * Controls what comparison operation is performed for depth testing.
         */
        CompareOperation depthTestingOperation = CompareOperation::NO_OP;

        /**
         * Enables whether depth bounds testing is performed; this involves
         * testing the depth values in the depth/stencil buffer using a
         * pair of depth bounds.
         */
        bool enableDepthBoundsTest = false;

        /**
         * This is the minimum depth bound used if depth bounds testing is enabled.
         */
        float depthBoundMinimumValue = 0.0f;

        /**
         * This is the maximum depth bound used if depth bounds testing is enabled.
         */
        float depthBoundMaximumValue = 0.0f;

        /**
         * Controls whether stencil testing is also enabled for a graphics pipeline.
         */
        bool enableStencilTest = false;

        /**
         * Configures stencil testing for front-facing render polygons if stencil testing is enabled.
         */
        StencilTestState frontFacingStencilTestState;

        /**
         * Configures stencil testing for back-facing render polygons if stencil testing is enabled.
         */
        StencilTestState backFacingStencilTestState;
    };

}