#pragma once

// taken right from scene::renderPass for now; specialize this in the future if needed

namespace pEngine::girEngine::gir::renderPass {

#define NUMBER_OF_CLEAR_VALUE_DIMENSIONS 4

/**
* For now, using Vulkan-style representation;
* Basically you specify exactly one of these arrays and it will be used
* to fill all pixels before rendering begins.
*
* Note that this value is only used when you specify the load operation to be
* CLEAR - else it is not used for anything.
*/
    struct ClearValue {
        std::vector<float> floatClearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        std::vector<int> intClearValue = {0, 0, 0, 0};
        std::vector<unsigned int> uintClearValue = {0, 0, 0, 0};
    };

}
