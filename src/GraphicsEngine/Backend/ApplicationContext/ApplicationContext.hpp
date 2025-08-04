//
// Created by paull on 2022-06-19.
//

#pragma once


namespace pEngine::girEngine::backend::appContext {

    /**
     * Honestly, the only thing here is the window system and then a bunch of vulkan-specific
     * stuff; maybe it makes sense to just leave this simple for now and then we can come back
     * and extend it as needed later on
     */
    class ApplicationContext {
    public:
        virtual ~ApplicationContext() = default;

    };

}