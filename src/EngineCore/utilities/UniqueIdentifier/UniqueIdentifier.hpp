//
// Created by paull on 2022-12-14.
//

#pragma once

#include <atomic>
#include <string>


namespace PUtilities {
    
    class UniqueIdentifier {
    public:
        UniqueIdentifier();

        UniqueIdentifier(const UniqueIdentifier &other) {
            value = other.value;
        }

        ~UniqueIdentifier() = default;

        // Would be good to hide the underlying type of the unique ID itself and instead
        // focus on providing an interface that does comparison
        bool operator==(const UniqueIdentifier &other) const;

        bool operator!=(const UniqueIdentifier &other) const;

        [[nodiscard]] unsigned long long int getValue() const;

        [[maybe_unused]] [[nodiscard]] std::string toString() const {
            return std::to_string(value);
        }

    private:
        static std::atomic<unsigned long long> globalUniqueIdCounter;

        unsigned long long value;

    };

} // PUtilities

namespace std {
    template<>
    struct hash<PUtilities::UniqueIdentifier> {
        std::size_t operator()(const PUtilities::UniqueIdentifier &uniqueIdentifier) const {
            return std::hash<unsigned long long>()(uniqueIdentifier.getValue());
        }
    };

    template<>
    struct hash<const PUtilities::UniqueIdentifier> {
        std::size_t operator()(const PUtilities::UniqueIdentifier &uniqueIdentifier) const {
            return std::hash<unsigned long long>()(uniqueIdentifier.getValue());
        }
    };
} // std
