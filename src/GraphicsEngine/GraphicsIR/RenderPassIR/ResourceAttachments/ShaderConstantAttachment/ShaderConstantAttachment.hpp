//
// Created by paull on 2023-10-20.
//

#pragma once

#include <memory>

namespace pEngine::girEngine::gir {
    class ShaderConstantIR;
}

namespace pEngine::girEngine::gir::renderPass {


    /**
     * This is an attachment of a particular shader/push constant to a render pass;
     * It'll be TODO until we get to backend bake
     */
    struct ShaderConstantAttachment {

        /**
         * The attached shader constant
         */
        std::shared_ptr<ShaderConstantIR> attachedShaderConstant = nullptr;
    };

} // renderPass
