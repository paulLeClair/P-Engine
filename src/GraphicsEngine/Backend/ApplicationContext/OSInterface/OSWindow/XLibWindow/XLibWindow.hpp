//
// Created by paull on 2024-06-13.
//

#pragma once


#include "../OSWindow.h"


#ifdef __linux__

#include <X11/Xlib.h>

namespace pEngine::girEngine::backend::appContext::osInterface::osWindow::xlib {

    class XLibWindow : public OSWindow {
    public:
        struct CreationInput {
            Display *display;

            unsigned initialWidth = 800;
            unsigned initialHeight = 600;

            unsigned int borderWidth;
            unsigned long border;
            unsigned long background;
        };

        explicit XLibWindow(const CreationInput &creationInput) {


            x11Window = XCreateSimpleWindow(
                    creationInput.display,
                    {}, // no parent
                    0,
                    0,
                    creationInput.initialWidth,
                    creationInput.initialHeight,
                    creationInput.borderWidth,
                    creationInput.border,
                    creationInput.background);
            if (!x11Window) {
                // TODO - log!
            }

            XSelectInput(creationInput.display, x11Window, 0); // not sure what to use for event mask
            XMapWindow(creationInput.display, x11Window);

            x11GraphicsContext = XCreateGC(creationInput.display, x11Window, 0, nullptr);

            XSetForeground(creationInput.display, x11GraphicsContext, windowCreationWhiteColor);

            // now apparently we have to wait for the MapNotify event (at which point the window is good to go?)
            while (true) {
                XEvent event;
                XNextEvent(creationInput.display, &event);

                if (event.type == MapNotify) {
                    break;
                }
            }
        }

        ~XLibWindow() = default;

        ExecuteWindowCommandResult executeWindowCommand(const WindowCommand &command) override {
            return ExecuteWindowCommandResult::FAILURE;
        }

        [[nodiscard]] unsigned int getWindowWidth() const override {
            return 0;
        }

        [[nodiscard]] unsigned int getWindowHeight() const override {
            return 0;
        }

        [[nodiscard]] Window getX11Window() const {
            return x11Window;
        }

    private:
        Window x11Window = {};

        GC x11GraphicsContext = {};

        int windowCreationBlackColor;
        int windowCreationWhiteColor;
    };

}

#endif