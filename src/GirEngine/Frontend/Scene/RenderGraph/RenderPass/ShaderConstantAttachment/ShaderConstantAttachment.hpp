//
// Created by paull on 2023-10-20.
//

#pragma once

namespace pEngine::girEngine::scene::graph::renderPass {
    class ShaderConstant;

    /**
     * This binds a push constant, called "shader constants" in this engine.
     */
    struct ShaderConstantAttachment {
        // this will be mostly todo until we get to backend bake...

        /**
         * The shader constant that's attached.
         */
        std::shared_ptr<ShaderConstant> attachedShaderConstant = nullptr;
    };
} // renderPass
