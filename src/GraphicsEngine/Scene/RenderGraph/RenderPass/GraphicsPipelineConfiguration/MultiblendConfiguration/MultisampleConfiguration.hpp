//
// Created by paull on 2023-10-26.
//

#pragma once

#include <cstdint>

namespace pEngine::girEngine::scene::graph::renderPass::pipeline {

    /**
     * All of this is ripped right out of the gir class (for now(?))
     */
    struct MultisampleConfiguration {
        /**
         * These denote the number of samples that should be taken per-pixel
         * if multisampling is enabled for this graphics pipeline.
         */
        enum class NumberOfSamples {
            UNSET = 0,
            ONE = 1,
            TWO = 2,
            FOUR = 4,
            EIGHT = 8,
            SIXTEEN = 16,
            THIRTY_TWO = 32,
            SIXTY_FOUR = 64,
        };

        /**
         * This configures whether multisampling is enabled for the pipeline.
         */
        bool enableMultisampling = false;

        /**
         * If multisampling is enabled, this denotes the number of samples to take per-pixel.
         */
        NumberOfSamples minimumNumberOfRasterizationSamples = NumberOfSamples::UNSET;

        /**
         * This enables or disables sample shading, where the user can set a minimum number of samples per fragment
         * to run fragment shaders on. Note that this factor is multiplied *in addition* to the minimum number
         * of rasterization samples, so if you enable this, the fragment shader will run a total of
         * \n\n
         * minimumNumberOfRasterizationSamples * fractionOfSamplesToShade
         * \n\n
         * times.
         */
        bool enableSampleShading = false;

        /**
         * This should be a value between 0 and 1 that denotes the number of samples that you want to shade.\n
         * Eg if you want to shade a minimum of 50% of the samples per fragment, use a value of 0.5
         */
        float fractionOfSamplesToShade = 0.0f;

        /**
         * This controls sample mask testing; it's TODO until I learn more about how that works
         */
        uint32_t sampleMask = 0;

        /**
         * Controls whether a "a temporary coverage value is generated based on the alpha component of the
         * fragment’s first color output as specified in the Multisample Coverage section [of the spec]"
         * according to the Vulkan spec - this is TODO until I learn more about it.
         */
        bool enableAlphaToCoverage = false;

        /**
         * Controls whether "controls whether the alpha component of the fragment’s first color output is replaced
         * with one" as described in the Multisample Coverage section of the spec.
         *
         * Taken straight from the Vulkan spec - this is TODO until I learn more about it.
         */
        bool enableAlphaToOne = false;
    };

} // pipeline
