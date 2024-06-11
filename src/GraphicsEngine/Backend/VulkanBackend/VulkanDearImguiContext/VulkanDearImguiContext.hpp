//
// Created by paull on 2023-11-17.
//

#pragma once

#include "../../../../lib/dear_imgui/imgui.h"

namespace pEngine::girEngine::backend::appContext::vulkan {

    class VulkanDearImguiContext {
    public:
        struct CreationInput {
            // here we'd put any vulkan / win32 configuration for DearImgui
            // (I'm actually not sure whether we can have this be agnostic to the backend as long as we include all the right lib files
        };

        explicit VulkanDearImguiContext(const CreationInput &creationInput) {
            imGuiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(imGuiContext);
        }

        [[nodiscard]] ImGuiContext *getImGuiContext() const {
            return imGuiContext;
        }

    private:
        ImGuiContext *imGuiContext;

    };

} // vulkan
