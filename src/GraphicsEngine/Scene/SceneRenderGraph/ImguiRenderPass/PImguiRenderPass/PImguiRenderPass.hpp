//
// Created by paull on 2022-11-16.
//

#pragma once

#include "../ImguiRenderPass.hpp"

namespace PGraphics {

    class PImguiRenderPass : public ImguiRenderPass {
    public:
        struct CreationInput {
            std::string name;
            std::vector<std::function<void()>> initialGuiElementCallbacks;
        };

        explicit PImguiRenderPass(const CreationInput &creationInput);

        void addGuiElementCallback(std::function<void()> &guiElementCallback);

        [[nodiscard]] const std::vector<std::function<void()>> &getGuiElementCallbacks() const {
            return guiElementCallbacks;
        }

    private:
        std::string name;

        std::vector<std::function<void()>> guiElementCallbacks = {};


    };

} // PGraphics
