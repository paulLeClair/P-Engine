//
// Created by paull on 2024-02-16.
//

#pragma once

#include <boost/describe/class.hpp>
#include <boost/optional/optional.hpp>

#include "Hash.hpp"

namespace pEngine::util::hash {
    /**
     * Not entirely sure what this will look like currently; the main idea is that
     * we need to allow objects to compute their own hash.
     *
     * One idea: make a class that you compose with whatever you want to hash which provides
     * a virtual function for computing the hash somehow, and then you just override that to
     * have the thing hash itself.
     *
     * And then we can probably just give it methods for getting the hash values
     *
     * Another potential thing: we may want to add some template argument(s),
     * the main thing I can think of now would be to choose the specific type you're using for the hash.
     * However, I don't see many cases where it would be beneficial to do that versus the default.
     *
     * TODO - private this hash value member and use a getter; we don't want it to be changed or anything.
     *
     * \brief
     */
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
