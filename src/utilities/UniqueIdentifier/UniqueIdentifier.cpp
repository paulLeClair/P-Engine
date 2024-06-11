//
// Created by paull on 2022-12-14.
//

#include "UniqueIdentifier.hpp"

namespace pEngine::util {
    boost::uuids::random_generator UniqueIdentifier::boostUuidGenerator = boost::uuids::random_generator_pure();
    //NOLINT

    UniqueIdentifier::UniqueIdentifier() : uniqueIdentifier(boostUuidGenerator()) {
    }

    bool UniqueIdentifier::operator==(const UniqueIdentifier &other) const {
        return other.uniqueIdentifier == uniqueIdentifier;
    }

    bool UniqueIdentifier::operator!=(const UniqueIdentifier &other) const {
        return !(*this == other);
    }

    boost::uuids::uuid UniqueIdentifier::getValue() const {
        return uniqueIdentifier;
    }
} // PUtilities
