//
// Created by paull on 2022-06-28.
//

#pragma once

namespace pEngine::girEngine::backend::appContext::osInterface {
    /**
     * This will be significantly reworked in the future, current design is just meant to get the ball rolling
     */
    class OSInterface {
    public:
        virtual ~OSInterface() = default;

        enum class WindowPlatform {
            WINDOWS,
            XLIB,
            XCB
        };

        struct CreateInfo {
            WindowPlatform windowPlatform; // TODO - change how this is specified
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
}
