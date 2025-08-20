//
// Created by paull on 2022-08-30.
//

#pragma once

namespace pEngine::app::mode {
    struct EngineMode {
        virtual ~EngineMode() = default;

        virtual void begin() = 0;
    };
}
