//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>
#include <vector>

class FrameExecutionController {
public:
    ~FrameExecutionController() = default;

    virtual void executeNextFrame() = 0;

    [[nodiscard]] virtual unsigned int getCurrentFrameIndex() const = 0;
};

