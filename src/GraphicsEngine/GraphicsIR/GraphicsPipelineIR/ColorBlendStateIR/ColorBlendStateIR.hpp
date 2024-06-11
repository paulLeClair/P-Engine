#pragma once

#include <vector>
#include <unordered_set>
#include "../../GraphicsIntermediateRepresentation.hpp"

namespace pEngine::girEngine::gir::pipeline {

    /**
    * This should contain information about how color blending is configured in the pipeline.
     *
     * I think for these smaller IR bits and bobs it's easiest to just use structs.
    */
    struct ColorBlendStateIR {
        ColorBlendStateIR() = default;

        /**
         * This configures whether color blending should be enabled at all.
         */
        bool enableColorBlend = false;

        /**
         * This controls what logic operation is done as part of standard color blending.
         */
        enum class LogicOperation {
            NO_OP,
            CLEAR,
            AND,
            AND_REVERSE,
            COPY,
            AND_INVERTED,
            XOR,
            OR,
            NOR,
            EQUIVALENT,
            INVERT,
            OR_REVERSE,
            COPY_INVERTED,
            OR_INVERTED,
            NAND,
            SET
        };

        enum class BlendFactor {
            ZERO,
            ONE,
            SRC_COLOR,
            ONE_MINUS_SRC_COLOR,
            DST_COLOR,
            ONE_MINUS_DST_COLOR,
            SRC_ALPHA,
            ONE_MINUS_SRC_ALPHA,
            DST_ALPHA,
            ONE_MINUS_DST_ALPHA,
            CONSTANT_COLOR,
            ONE_MINUS_CONSTANT_COLOR,
            CONSTANT_ALPHA,
            ONE_MINUS_CONSTANT_ALPHA,
            SRC_ALPHA_SATURATE,
            SRC1_COLOR,
            ONE_MINUS_SRC1_COLOR,
            SRC1_ALPHA,
            ONE_MINUS_SRC1_ALPHA
        };

        enum class BlendOperation {
            NO_OP,
            ADD,
            SUBTRACT,
            REVERSE_SUBTRACT,
            MIN,
            MAX
            // TODO - add options that leverage the advanced blending operations Vulkan extension
        };

        enum ColorComponent {
            RED = 0,
            GREEN = 1,
            BLUE = 2,
            ALPHA = 3
        };

        /**
         * When color blending is enabled, this controls the color blending
         * behavior for a particular color attachment.
         */
        struct ColorAttachmentBlendState {
            /**
             * This controls whether color blend is enabled at all for this attachment.
             */
            bool enableColorBlendingForThisAttachment = false;

            /**
             * Controls the source RGB blend factor (used in the color blending equation) to be used
             * for this color attachment, commonly labeled S. \n\n
             *
             * Only the enabled components provided will have the blend operation applied to them;
             * by default all components will be enabled. \n\n
             *
             * Note that this doesn't include the alpha component which
             * is configured in another field inside ColorAttachmentBlendState.
             */
            BlendFactor sourceColorBlendFactor = BlendFactor::ZERO;

            /**
             * Controls the destination RGB blend factor (used in the color blending equation)
             * to be used for this color attachment, commonly labeled D. \n\n
             *
             * Only the enabled components provided will have the blend operation applied to them;
             * by default all components will be enabled. \n\n
             *
             * Does not include the alpha component.
             */
            BlendFactor destinationColorBlendFactor = BlendFactor::ZERO;

            /**
             * Controls what blend operation is performed for the RGB components
             */
            BlendOperation colorBlendOp = BlendOperation::NO_OP;

            /**
             * Controls the source alpha blending factor.
             */
            BlendFactor sourceAlphaBlendFactor = BlendFactor::ZERO;

            /**
             * Controls the destination alpha blending factor
             */
            BlendFactor destinationAlphaBlendFactor = BlendFactor::ZERO;

            /**
             * Controls the blending operation performed for the alpha component
             */
            BlendOperation alphaBlendOp = BlendOperation::NO_OP;

            /**
             * Controls which color blend components are enabled; all are enabled by default
             */
            std::unordered_set<ColorComponent> enabledBlendComponents = {RED, GREEN, BLUE, ALPHA};
        };

        /**
         * Controls whether a configurable logic operation will be
         * performed as part of color blending.
         */
        bool enableColorBlendLogicOperation = false;

        /**
         * Sets which logic operation is to be used when color blending
         * and color blending logic operation are enabled.
         */
        LogicOperation logicOp = LogicOperation::NO_OP;

        /**
         * This vector should hold the RGBA constant values to be used
         * if the blend factor is meant to use
         */
        std::vector<float> blendConstants = {0.0, 0.0, 0.0, 0.0};

        /**
         * When color blending is enabled, this should be filled with one
         * instance for each color attachment and will specify how
         * color blending will happen for that color attachment.
         *
         * The backend should complain if these do not match up with
         * the render pass' color attachment state.
         */
        std::vector<ColorAttachmentBlendState> colorAttachmentBlendStates = {};
    };

}