//
// Created by paull on 2024-09-23.
//

#pragma once


#include "../../ResourceIR/FormatIR/FormatIR.hpp"

namespace pEngine::girEngine::gir::pipeline {
    struct DynamicRenderPassPipelineStateIR {
        using UidFormatPair = std::pair<util::UniqueIdentifier, resource::FormatIR>;
        std::vector<UidFormatPair> colorAttachmentFormats = {};

        UidFormatPair depthStencilAttachmentFormat = {};

        // view mask (unused until multiview is supported)
        unsigned viewMask = 0;
    };
}
