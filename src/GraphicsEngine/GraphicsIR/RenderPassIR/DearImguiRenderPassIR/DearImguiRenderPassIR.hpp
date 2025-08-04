//
// Created by paull on 2024-05-29.
//

#pragma once


#include "../RenderPassIR.hpp"

namespace pEngine::girEngine::gir::renderPass::dearImgui {

    class DearImguiRenderPassIR : public RenderPassIR {
    public:
        struct CreationInput : RenderPassIR::CreationInput {
            std::vector<std::function<void()>> dearImguiGuiCallbacks = {};
        };

        explicit DearImguiRenderPassIR(const CreationInput &creationInput)
                : RenderPassIR(creationInput), dearImguiGuiCallbacks(creationInput.dearImguiGuiCallbacks) {

        }

        DearImguiRenderPassIR() : RenderPassIR({}), dearImguiGuiCallbacks() {

        }

        [[nodiscard]] const std::vector<std::function<void()>> &getDearImguiGuiCallbacks() const {
            return dearImguiGuiCallbacks;
        }


    private:
        // TODO - anything that we need to store here in the GIR lol
        std::vector<std::function<void()>> dearImguiGuiCallbacks = {};
    };

} // dearimgui
