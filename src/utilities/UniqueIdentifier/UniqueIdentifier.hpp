//
// Created by paull on 2022-12-14.
//

#pragma once

#include <boost/uuid/uuid_generators.hpp>
#include <boost/functional/hash.hpp>

namespace pEngine::util {
    /**
     * Maybe a slightly-less baby-brain adjustment to the baby-brain current design: just basically use this to
     * wrap a boost uuid since we added boost anyway. The boost one is probably more robust and performant anyway,
     * having an atomic is not the most performant way to do this so I'll give that a shot for now
     */
    class UniqueIdentifier {
    public:
        UniqueIdentifier();

        UniqueIdentifier(const UniqueIdentifier &other) = default;

        ~UniqueIdentifier() = default;

        bool operator==(const UniqueIdentifier &other) const;

        bool operator!=(const UniqueIdentifier &other) const;

        [[nodiscard]] boost::uuids::uuid getValue() const;

    private:
        static boost::uuids::random_generator boostUuidGenerator;

        boost::uuids::uuid uniqueIdentifier;
    };
}

namespace std {
    template<>
    struct hash<boost::uuids::uuid> {
        size_t operator()(const boost::uuids::uuid &uid) const noexcept {
            return boost::hash<boost::uuids::uuid>()(uid);
        }
    };

    template<>
    struct hash<pEngine::util::UniqueIdentifier> {
        size_t operator()(const pEngine::util::UniqueIdentifier &uid) const noexcept {
            return std::hash<boost::uuids::uuid>()(uid.getValue());
        }
    };
}
