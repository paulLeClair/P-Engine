//
// Created by paull on 2022-12-01.
//

#pragma once


class Frame {
public:
    ~Frame() = default;

    virtual void execute() = 0;

    [[nodiscard]] virtual unsigned int getFrameIndex() const = 0;

};
