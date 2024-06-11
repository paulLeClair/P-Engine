//
// Created by paull on 2022-06-19.
//

#pragma once


#include "OSInterface/OSInterface.hpp"

namespace pEngine::girEngine::backend::appContext {

    /**
     * Okay, let's get back to this stuff I guess;
     *
     * I'm still not entirely sure how reasonable/cross-API of a design choice it is to break off
     * the application context into its own thing like this... seems like it should work though.
     *
     * Worst case I guess we just pull out the window system to be on the top level of the Backend directory/namespace?
     *
     * Honestly, the only thing here is the window system and then a bunch of vulkan-specific
     * stuff; maybe it makes sense to just leave this simple for now and then we can come back
     * and extend it as needed later on
     */
    class ApplicationContext {
    public:
        virtual ~ApplicationContext() = default;

    };

}