//
// Created by paull on 2022-06-18.
//

#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "../ShaderModule.hpp"

namespace PGraphics {

    class PShaderModule : public ShaderModule {
    public:
        explicit PShaderModule(const CreateInfo &info) : ShaderModule(info) {

        }

    private:
        
    };

}// namespace PGraphics
