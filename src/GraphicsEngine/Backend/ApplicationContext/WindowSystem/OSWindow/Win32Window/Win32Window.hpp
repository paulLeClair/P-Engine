//
// Created by paull on 2022-08-24.
//

#pragma once

#include <Windows.h>

#include <string>
#include <memory>

#include "../../../../../../lib/dear_imgui/imgui.h"
#include "../../../../../../lib/dear_imgui/imgui_impl_vulkan.h"
#include "../../../../../../lib/dear_imgui/imgui_impl_win32.h"
#include "../../../../../../lib/dear_imgui/imgui_internal.h"
#include "../OSWindow.h"
#include "../../WindowSystem.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace PGraphics {

    class Win32Window : public OSWindow {
    public:
        struct CreationInput {
            std::string windowClassName;

            std::string windowText;

            WindowSystem *windowSystemPtr = nullptr;

            // we'll probably need a connection to ImGui here - seems ugly but I'm not sure how else to do it
            [[maybe_unused]] bool isImGuiEnabled = true;
        };

        explicit Win32Window(const CreationInput &createInfo);

        ~Win32Window() = default;

        ExecuteWindowCommandResult executeWindowCommand(const WindowCommand &command) override;

        [[nodiscard]] HWND getWinApiHWND() const;

    private:
        HINSTANCE win32Instance;

        HWND win32Window;
        //        HRGN win32WindowRegion;

        WindowSystem *windowSystemPtr = nullptr;

        static LRESULT
        windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static void WindowProc_resize(HWND hwnd, UINT wParam, int width, int height);

        static int getWin32ShowCommand(const WindowCommand &command);
    };

}// namespace PGraphics
