//
// Created by paull on 2025-05-16.
//

#pragma once
#include <functional>

namespace pEngine::util::gui {
    /**
     * Idea: this will wrap the implementation-independent DearImgui library callback stuff; I want this to be a
     * general purpose sub-library which allows you to conveniently build up lil interfaces without having to
     * look into the specifics of DearImgui (the goal is to simplify everything down so that it's as painless to use as possible)
     *
     */
    class GuiWidget {
    public:
        virtual ~GuiWidget() = default;

    private:
        /**
         *
         * @return A callable void() function that will execute the DearImgui commands to render the widget
         */
        virtual std::function<void()> toCallback() = 0;
    };
}
