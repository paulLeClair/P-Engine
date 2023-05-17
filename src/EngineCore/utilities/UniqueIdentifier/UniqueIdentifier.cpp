//
// Created by paull on 2022-12-14.
//

#include "UniqueIdentifier.hpp"

namespace PUtilities {

    std::atomic<unsigned long long> UniqueIdentifier::globalUniqueIdCounter = 0;

    UniqueIdentifier::UniqueIdentifier() {
        value = globalUniqueIdCounter.load();
        globalUniqueIdCounter.fetch_add(1);
    }

    bool UniqueIdentifier::operator==(const UniqueIdentifier &other) const {
        return other.value == value;
    }

    bool UniqueIdentifier::operator!=(const UniqueIdentifier &other) const {
        return !(*this == other);
    }

    unsigned long long int UniqueIdentifier::getValue() const {
        return value;
    }

} // PUtilities