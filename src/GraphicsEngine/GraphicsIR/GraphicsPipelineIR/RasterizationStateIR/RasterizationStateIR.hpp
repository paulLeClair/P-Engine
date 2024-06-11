#pragma once

#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::pipeline {

    /**
     * This should contain the information to control the rasterization happening inside the graphics pipeline
    */
    struct RasterizationStateIR {

        struct ExtendedDepthClipState {
            // TODO until we get basic scenes rendering
        };

        /**
         * These are possible configurations for how the rasterizer will actually treat the primitives you
         * give it.
         */
        enum class PolygonRasterizationMode {
            /**
             * Rasterize polygons as normal, coloring the interiors of polygons
             */
            FILL,
            /**
             * Rasterize all polygon vertices as points.
             */
            POINTS,
            /**
             * Rasterize all polygon edges as lines.
             */
            LINES
        };

        /**
         * These are possible configurations for how fixed-function polygon culling will work.
         */
        enum class PolygonCullingMode {
            /**
             * No polygons are culled
             */
            NONE,
            /**
             * Front-facing polygons are culled (be careful!).
             */
            FRONT_FACING,
            /**
             * Back-facing polygons are culled
             */
            BACK_FACING,
            /**
             * All polygons are culled (both front and back facing)
             */
            ALL
        };

        /**
         * Possible configurations for which direction of traversal (of the vertices) is used to determine
         * which way polygons are facing.
         */
        enum class PolygonFrontFaceOrientationMode {
            CLOCKWISE,
            COUNTER_CLOCKWISE
        };

        /**
         * Adapted from the Vulkan spec and is TODO until I learn more and flesh out how this is configured.
         *\n\n
         * This setting controls "whether to clamp the fragment’s depth values as described in Depth Test" in the spec.
         *\n\n
         * If the pipeline is not created with enableExtendedDepthClipState set to true then
         * enabling depth clamp will also disable clipping primitives to the z planes of the frustrum as described
         * in Primitive Clipping (in the spec). Otherwise depth clipping is controlled by the state set
         * in ExtendedDepthClipState.
         */
        bool enableDepthClamping = false;

        /**
         * This specifies whether you want to make use of primitive clipping and any other
         * fancy depth / clipping configuration stuff. TODO until we get more basic stuff running.
         */
        bool enableExtendedDepthClipState = false;

        /**
         * This is where you configure fancier depth/clip state, TODO until we get more basic stuff running.
         */
        ExtendedDepthClipState extendedDepthClipState = {};


        /**
         * This determines whether the user wants to basically disable rasterization by discarding all primitives
         * first before the pipeline stage begins. BE CAREFUL USING THIS! This is only for very particular purposes.
         */
        bool discardAllPrimitivesBeforeRasterization = false;

        PolygonRasterizationMode rasterizationMode = PolygonRasterizationMode::FILL;

        PolygonCullingMode cullingMode = PolygonCullingMode::NONE;

        PolygonFrontFaceOrientationMode polygonFrontFaceOrientation = PolygonFrontFaceOrientationMode::CLOCKWISE;

        /**
         * If polygon rasterization mode is set to display lines, this controls the width of those lines.
         */
        float lineWidth = 0.0f;

        /**
         * This controls whether a configurable depth bias value is added to fragments' depth values.
         */
        bool enableDepthBias = false;

        /**
         * Controls the depth value that's **added** to each fragment's depth if depth bias is enabled
         */
        float depthBiasAdditiveConstantValue = 0.0f;

        /**
         * Maximum or minimum (somehow?) depth bias of a fragment if depth bias is enabled.\n\n
         * Not sure how it's determined to be one or the other.
         */
        float depthBiasClamp = 0.0f;

        /**
         * If depth bias is enabled, this is a scalar factor controlling the additive constant depth value added to each fragment.
         * I'm assuming that means the actual depth value added is given by: \n\n
         *
         * depthBiasAdditiveConstantValue * depthBiasSlopeFactor
         */
        float depthBiasSlopeFactor = 0.0f;
    };

}