//
// Created by paull on 2022-08-24.
//

#include "Win32Window.hpp"

#include <stdexcept>

namespace PGraphics {
    LRESULT Win32Window::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        ImGuiIO &io = ImGui::GetIO();

        auto imguiWindowProcResult = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        if (io.WantCaptureMouse &&
            (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP ||
             uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP || uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEMOVE)) {
            return TRUE;
        }

        switch (uMsg) {
            case WM_SIZE:
            case WM_EXITSIZEMOVE:
                //   PostMessage( hwnd, USER_MESSAGE_RESIZE, wParam, lParam );
            {
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);

                WindowProc_resize(hWnd, (UINT) wParam, width, height);

                // acquire windowsystem pointer attribute from the HWND
                auto *windowSystem = reinterpret_cast<WindowSystem *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
                windowSystem->resizeWindow((unsigned int) width, (unsigned int) height);
            }
                break;
            case WM_KEYDOWN:
                if (VK_ESCAPE == wParam) {
                    PostQuitMessage(0);
                }
                break;
            case WM_CLOSE:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        return 0;
    }

    void Win32Window::WindowProc_resize(HWND hwnd, UINT wParam, int width, int height) {
        // TODO: flesh this out/look into it more
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

    Win32Window::Win32Window(const CreationInput &createInfo) {
        const auto CLASS_NAME = reinterpret_cast<LPCSTR const>( createInfo.windowClassName.c_str());

        if (createInfo.windowSystemPtr == nullptr) {
            throw std::runtime_error("Unable to create Win32Window: Provided pointer to window system is null!");
        }
        windowSystemPtr = createInfo.windowSystemPtr;

        win32Instance = GetModuleHandle(nullptr);

        WNDCLASS windowClass = {};

        windowClass.lpfnWndProc = this->windowProc;
        windowClass.hInstance = win32Instance;
        windowClass.lpszClassName = CLASS_NAME;

        RegisterClass(&windowClass);

        win32Window = CreateWindowEx(0,
                                     CLASS_NAME,
                                     reinterpret_cast<LPCSTR const>( createInfo.windowText.c_str()),
                                     WS_OVERLAPPEDWINDOW,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     nullptr,
                                     nullptr,
                                     win32Instance,
                                     nullptr);

        if (win32Window == nullptr) {
            auto lastError = GetLastError();
            throw std::runtime_error(
                    "Unable to create Win32 HWND! Last Win32 error number: " + std::to_string(lastError));
        }

        // set HWND property for windowSystem pointer

        // ... have to call SetLastError(0) to be able to determine whether this succeeds or fails?
        // win32 more like wtf32, now I have to leave dumb comments like this to make this code make sense
        SetLastError(0);

        if (!SetWindowLongPtr(win32Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowSystemPtr))) {
            auto lastError = GetLastError();
            if (lastError != 0) {
                throw std::runtime_error(
                        "Unable to set HWND's WindowSystem pointer attribute! Last Win32 error number: " +
                        std::to_string(lastError));
            }
        }

        // have to call SetWindowPos to make changes effective
        if (!SetWindowPos(win32Window, nullptr, 0, 0, 0, 0,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED)) {
            auto lastError = GetLastError();
            throw std::runtime_error(
                    "Call to SetWindowPos failed during Win32Window creation! Last Win32 error number: " +
                    std::to_string(lastError));
        }
    }

    OSWindow::ExecuteWindowCommandResult Win32Window::executeWindowCommand(const WindowCommand &command) {
        try {
            int showCommand = getWin32ShowCommand(command);
            ShowWindow(win32Window, showCommand);
        } catch (std::exception &exception) {
            return OSWindow::ExecuteWindowCommandResult::FAILURE;
        }

        return OSWindow::ExecuteWindowCommandResult::SUCCESS;
    }

    int Win32Window::getWin32ShowCommand(const OSWindow::WindowCommand &command) {
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
            default:
                throw std::runtime_error("Unable to determine Win32 command type!");
        }

        return commandInteger;
    }

    HWND Win32Window::getWinApiHWND() const {
        return win32Window;
    }
}// namespace PGraphics