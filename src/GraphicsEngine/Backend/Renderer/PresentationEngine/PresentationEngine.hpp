//
// Created by paull on 2024-05-23.
//

#pragma once


namespace pEngine::girEngine::backend {

    class PresentationEngine {
    public:
        virtual ~PresentationEngine() = default;

        virtual boost::optional<unsigned> acquireNextSwapchainImageIndex() = 0;

    };
}
