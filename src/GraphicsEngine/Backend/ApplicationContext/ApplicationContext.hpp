//
// Created by paull on 2022-06-19.
//

#pragma once


#include "WindowSystem/WindowSystem.hpp"

class ApplicationContext {
public:
    ~ApplicationContext() = default;

    virtual std::shared_ptr<WindowSystem> getWindowSystem() = 0;
};
