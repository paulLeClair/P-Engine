#pragma once


namespace pEngine::girEngine::frontend {
    struct BakeOutput;
}

namespace pEngine::girEngine::backend {
    class Backend {
    public:
        virtual ~Backend() = default;

        enum class DrawFrameResult {
            SUCCESS,
            FAILURE
        };

        virtual DrawFrameResult drawFrame() = 0;

        enum class ConsumeResult {
            FAILURE,
            SUCCESS
        };

        virtual ConsumeResult
        consumeGirs(const frontend::BakeOutput &girList) = 0;
    };
}
