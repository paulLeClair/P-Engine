//
// Created by paull on 2022-08-30.
//

#pragma once

class EngineMode {
public:
    ~EngineMode() = default;

    // TODO - re-evaluate the engine mode interface...

    enum class RunResult {
        SUCCESS,
        FAILURE
    };

    virtual RunResult run() = 0;

};