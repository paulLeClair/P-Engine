//
// Created by paull on 2023-12-18.
//

#pragma once

// TODO - probably make a pEngine::util::hash namespace and then specialize further from there
namespace pEngine::util {

    /**
     * I'm fairly certain this can be simple for now; I might just try and make it wrap the std::hash for now
     * and then in the future I can look into more custom implementations for this engine.
     */
    using GlobalEngineHashType = size_t; // until I implement my own hashing stuff, we'll just align it with std::hash
    using Hash = GlobalEngineHashType; // making it extra clear what this abbreviation really means

    // TODO - write custom hash/hashers when needed


} // util
