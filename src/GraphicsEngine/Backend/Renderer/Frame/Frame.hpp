//
// Created by paull on 2022-12-01.
//

#pragma once

namespace pEngine::girEngine::backend::frameController::frame::vulkan {

    class Frame {
    public:
        ~Frame() = default;

        virtual void execute() = 0;

        [[nodiscard]] virtual unsigned int getFrameIndex() const = 0;

    };

}