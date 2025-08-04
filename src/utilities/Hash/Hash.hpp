#pragma once

namespace pEngine::util {

    using GlobalEngineHashType = size_t; // until I implement my own hashing stuff, we'll just align it with std::hash
    using Hash = GlobalEngineHashType; // making it extra clear what this abbreviation really means

    // TODO - write custom hash/hashers when needed

} // util
