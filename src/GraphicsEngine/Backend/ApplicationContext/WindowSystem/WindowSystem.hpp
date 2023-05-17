//
// Created by paull on 2022-06-28.
//

#pragma once

class WindowSystem {
public:
    ~WindowSystem() = default;

    enum class WindowPlatform {
        WINDOWS,
        XLIB,
        XCB
    };

    struct CreateInfo {
        WindowPlatform windowPlatform;

        explicit CreateInfo(WindowPlatform windowPlatform) : windowPlatform(windowPlatform) {

        }
    };

    [[nodiscard]] virtual WindowPlatform getWindowPlatform() const = 0;

    enum class CreateWindowResult {
        SUCCESS,
        FAILURE
    };

    virtual CreateWindowResult createWindow() = 0;

    enum class ShowWindowResult {
        SUCCESS,
        FAILURE
    };

    virtual ShowWindowResult showWindow() = 0;

    enum class ResizeWindowResult {
        SUCCESS,
        FAILURE
    };

    virtual ResizeWindowResult resizeWindow(unsigned int length, unsigned int width) = 0;
};
