//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>
#include <vector>

namespace pEngine::girEngine::backend {
    /**
     * We're finally here!
     *
     * I think it might make sense to rename this whole "Renderer" thing (which I don't like as much now)
     * to something like a more general "Renderer" or something;
     *
     * If you think about it, the main purpose of this whole abstraction is to just take the stuff that was built up
     * at bake time and start drawing frames according to however it has been configured.
     *
     * It's where the actual rendering happens, and I think that's the main reason it's separated from the app context
     * in the first place.
     *
     * So yeah fuck it I'm goin for it
     */
    class Renderer {
    public:
        virtual ~Renderer() = default;


    };
}
