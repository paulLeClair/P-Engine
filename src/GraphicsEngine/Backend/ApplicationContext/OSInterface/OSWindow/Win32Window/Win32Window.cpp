//
// Created by paull on 2022-08-24.
//

#include "Win32Window.hpp"

#ifdef _WIN32

#include <stdexcept>

namespace pEngine::girEngine::backend::appContext::osInterface::osWindow::win32 {
    LRESULT Win32Window::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
            return TRUE;
        }

        switch (uMsg) {
            // TODO - sort this hideous win32 api stuff out. For now I'm just looking for bare minimum
            default:
                return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
    }

    void Win32Window::WindowProc_resize(HWND hwnd, UINT wParam, int width, int height) {
        // TODO: flesh this out/look into it more;
        // I think we do need to be doing something with the width/height thing,
        // the main thing I'm thinking of is resizing the swapchain to the new extent given by width and height
        switch (wParam) {
            case SIZE_MAXHIDE:
                ShowWindow(hwnd, SW_MINIMIZE);
                break;

            case SIZE_MAXIMIZED:
                ShowWindow(hwnd, SW_MAXIMIZE);
                break;

            case SIZE_MINIMIZED:
                ShowWindow(hwnd, SW_MINIMIZE);
                break;

            case SIZE_RESTORED:
                ShowWindow(hwnd, SW_RESTORE);
                break;

            case SIZE_MAXSHOW:
                ShowWindow(hwnd, SW_MINIMIZE);
                break;
            default:
                throw std::runtime_error("Unable to determine wParam value!");
        }
    }

    Win32Window::Win32Window(const CreationInput &createInfo) : windowText(createInfo.windowText) {
        const auto CLASS_NAME = reinterpret_cast<LPCSTR const>(createInfo.windowClassName.c_str());

        if (createInfo.windowSystemPtr == nullptr) {
            throw std::runtime_error("Unable to create Win32Window: Provided pointer to window system is null!");
        }
        windowSystemPtr = createInfo.windowSystemPtr;

        win32Instance = GetModuleHandle(nullptr);

        WNDCLASS windowClass = {};

        windowClass.lpfnWndProc = windowProc;
        windowClass.hInstance = win32Instance;
        windowClass.lpszClassName = CLASS_NAME;

        RegisterClass(&windowClass);


        win32Window = CreateWindowEx(0,
                                     CLASS_NAME,
                                     (LPCSTR) L"GirEngine Demo",
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     createInfo.initialWindowWidth,
                                     createInfo.initialWindowHeight,
                                     nullptr,
                                     nullptr,
                                     win32Instance,
                                     nullptr);

        if (win32Window == nullptr) {
            auto lastError = GetLastError();
            throw std::runtime_error(
                "Unable to create Win32 HWND! Last Win32 error number: " + std::to_string(lastError));
        }

        // finally, store width and height (TODO - ensure this gets updated when resizing (not implemented yet))
        RECT rect;
        if (GetWindowRect(getWinApiHWND(), &rect)) {
            windowWidth = rect.right - rect.left;
            windowHeight = rect.bottom - rect.top;
        }
    }

    OSWindow::ExecuteWindowCommandResult Win32Window::executeWindowCommand(const WindowCommand &command) {
        try {
            int showCommand = getWin32ShowCommand(command);
            ShowWindow(win32Window, showCommand);
        } catch (std::exception &exception) {
            return ExecuteWindowCommandResult::FAILURE;
        }

        return ExecuteWindowCommandResult::SUCCESS;
    }

    int Win32Window::getWin32ShowCommand(const WindowCommand &command) {
        int commandInteger = -1;

        switch (command) {
            case (WindowCommand::HIDE): {
                commandInteger = SW_HIDE;
                break;
            }
            case (WindowCommand::MAXIMIZE): {
                commandInteger = SW_MAXIMIZE;
                break;
            }
            case (WindowCommand::MINIMIZE): {
                commandInteger = SW_MINIMIZE;
                break;
            }
            case (WindowCommand::SHOW): {
                commandInteger = SW_SHOW;
                break;
            }
            // TODO - support any other commands as applicable
            default:
                throw std::runtime_error("Unable to determine Win32 command type!");
        }

        return commandInteger;
    }

    HWND Win32Window::getWinApiHWND() const {
        return win32Window;
    }
} // namespace PGraphics

#endif
