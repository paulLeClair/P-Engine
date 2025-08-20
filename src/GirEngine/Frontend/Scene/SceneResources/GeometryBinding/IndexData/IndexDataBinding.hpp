//
// Created by paull on 2024-10-01.
//

#pragma once

namespace pEngine::girEngine::scene::geometry {

    struct IndexDataBinding {
        // not sure how much info we really need here; i guess
        // for the time being I'll just have it specify an index type
        enum class IndexType {
            UNDEFINED,
            // are these the only relevant options really?
            INDEX_16_BIT,
            INDEX_32_BIT
        };
        IndexType type = IndexType::INDEX_32_BIT;

    };

}
