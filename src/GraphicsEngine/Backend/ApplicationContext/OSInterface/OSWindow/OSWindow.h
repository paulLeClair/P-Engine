//
// Created by paull on 2022-08-24.
//

#pragma once

namespace pEngine::girEngine::backend::appContext::osInterface::osWindow {

    class OSWindow {
    public:
        ~OSWindow() = default;

        enum class ExecuteWindowCommandResult {
            SUCCESS,
            FAILURE
        };

        enum class WindowCommand {
            SHOW,
            MINIMIZE,
            MAXIMIZE,
            HIDE
            // and so on...?
        };

        virtual ExecuteWindowCommandResult executeWindowCommand(const WindowCommand &command) = 0;

        [[nodiscard]] virtual unsigned getWindowWidth() const = 0;

        [[nodiscard]] virtual unsigned getWindowHeight() const = 0;
    };

}