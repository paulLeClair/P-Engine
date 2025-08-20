#pragma once

// taken right from scene::renderPass for now; specialize this in the future if needed

namespace pEngine::girEngine::gir::renderPass {

    enum class MultisampleResolveOperation {
        /**
         * No resolve operation is done
        */
        NO_OP,
        /**
         * This resolve operation means that it will set all values to the
         * first sample (sample 0)
        */
        SET_TO_FIRST_SAMPLE,
        /**
         * The output of the resolve operation is the average of all sample values
        */
        AVERAGE_OF_ALL_SAMPLES,
        /**
         * The output of the resolve operation is the minimum sample value
        */
        MINIMUM_SAMPLE,
        /**
         * The output of the resolve operation is the maximum sample value
        */
        MAXIMUM_SAMPLE
    };

}

