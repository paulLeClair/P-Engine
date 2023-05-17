//
// Created by paull on 2022-11-16.
//

#include "PImguiRenderPass.hpp"

namespace PGraphics {

    PImguiRenderPass::PImguiRenderPass(const PImguiRenderPass::CreationInput &creationInput) : name(creationInput.name),
                                                                                               guiElementCallbacks(
                                                                                                       creationInput.initialGuiElementCallbacks) {

    }

    void PImguiRenderPass::addGuiElementCallback(std::function<void()> &guiElementCallback) {
        guiElementCallbacks.push_back(guiElementCallback);
    }


} // PGraphics