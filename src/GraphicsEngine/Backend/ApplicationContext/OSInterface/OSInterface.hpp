//
// Created by paull on 2022-06-28.
//

#pragma once

namespace pEngine::girEngine::backend::appContext::osInterface {

    /**
     * Returning to this after a long time:
     *
     * I think a lot of the functionality here will be good, but it seems like the window system is doing
     * more than just creating a window; it's also doing the crucial step of processing input from the OS.
     *
     * I think, to clarify what this class does as well as provide an umbrella to put other chunks underneath,
     * we could change this to something like "OperatingSystemIntegration" or "OperatingSystemInterface"
     *
     * This way it can handle inputs just fine, and then hopefully we can separate out (in an OS-independent way)
     * the act of obtaining actual input from the OS and the actual processing of inputs; we can have a generic interface
     * that is designed to be mapped onto an actual OS interface.
     *
     * My goal is to design things to save time in the future, and also to be broken up into small chunks with well-defined jobs.
     *
     * So we already have the windowing shit put in, now I guess we just have to wire up the swapchain stuff
     * so we can start presenting images. Then we can probably do the present mode abstraction once I've reviewed a bit
     * of how that all works (I should probably read the spec for a bit to refresh my memory)
     *
     */
    class OSInterface {
    public:
        ~OSInterface() = default;

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