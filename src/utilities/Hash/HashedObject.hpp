//
// Created by paull on 2024-02-16.
//

#pragma once

#include "Hash.hpp"

namespace pEngine::util::hash {

    struct HashedObject {
        HashedObject() = default;

        virtual ~HashedObject() = default;

        /**
         * \brief This should just allow an inheriting class to compute and store its own hash.
         * \return the new hash value
         */
        virtual Hash computeHash(Hash seed) = 0;

        friend bool operator==(const HashedObject &lhs, const HashedObject &rhs) {
            return lhs.hashValue == rhs.hashValue;
        }

        friend bool operator!=(const HashedObject &lhs, const HashedObject &rhs) {
            return !(lhs == rhs);
        }

        Hash hashValue = 0u;
    };
} // pEngine
