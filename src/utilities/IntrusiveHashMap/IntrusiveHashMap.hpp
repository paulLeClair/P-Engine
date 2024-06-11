//
// Created by paull on 2023-12-18.
//

#pragma once

#include "../IntrusiveList/IntrusiveList.hpp"
#include "../Hash/Hash.hpp"

#include <utility>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include <boost/container/stable_vector.hpp>

#define MAX_INSERT_ATTEMPTS 100u
#define MAX_FIND_ATTEMPTS 10u

namespace pEngine::util {
    /**
     * This class is meant to store the hash and give any inheriting class a util::Hash
     * and allow it to be an IntrusiveList element storing ValueType;
     * Note this class does *not* actually store the data (which has type ValueType).
     * Like Themaister's, I think this should be non-owning; it just manages a bunch of pointers.
     * @tparam ValueType
     */
    template<typename ValueType>
    struct IntrusiveHashMapEntry : public IntrusiveListNode<ValueType> {
        IntrusiveHashMapEntry() = default;

        explicit IntrusiveHashMapEntry(const util::Hash hash_)
            : hash(hash_) {
        }

        void setHash(const util::Hash newHash) {
            hash = newHash;
        }

        [[nodiscard]] util::Hash getHash() const {
            // NOLINT
            return hash;
        }

    private:
        util::Hash hash = 0;
    };

    /**
     * This is meant to be used as a base class for any ValueType that you want to
     * store in the IntrusiveHashMap (recall that we're handling hashes separately unlike STL design)
     *
     * OKAY - JAN13 -> I think the problem is that we're trying to use raw intrusive list iterators
     * in the object pool's allocationMap; but the Iterator is not a subclass of IntrusiveHashMapEntry<>!
     *
     * This is why Themaister's implementation uses this POD wrapper thing; I'm not entirely sure of how you're supposed
     * to use his implementation but I can definitely adapt it (which should hopefully fix some issues with the
     * object pool and if we're lucky will help me get past this/these goddamn bug(s) I've been stuck on
     *
     * @tparam ValueType
     */
    template<typename ValueType>
    struct IntrusiveHashMapPODWrapper : public IntrusiveHashMapEntry<IntrusiveHashMapPODWrapper<ValueType> > {
        // Themaister-style constructor with std::forward<T>() -> forwards the value as either an lvalue
        // or rvalue depending on T (still a little foggy on the specifics)
        template<typename T>
        explicit IntrusiveHashMapPODWrapper(T &&value) : entryValue(std::forward<T>(value)) {
            //NOLINT
        }

        IntrusiveHashMapPODWrapper() = default;

        ValueType &get() {
            return entryValue;
        }

        const ValueType &get() const {
            return entryValue;
        }

        ValueType entryValue = {};
    };

    /**
     * OWNING intrusive hash map
     * @tparam ValueType
     */
    template<typename ValueType>
    class IntrusiveHashMap {
    public:
        explicit IntrusiveHashMap() {
        }

        IntrusiveHashMap(const IntrusiveHashMap<ValueType> &other) = delete;

        void operator=(const IntrusiveHashMap<ValueType> &other) = delete;

        /**
         * Returns a pointer to a ValueType that is stored inside the IntrusiveHashMap.
         * If no ValueType could be found for the given hash, or if the map is empty, it will return nullptr
         * @param hash
         * @return
         */
        ValueType *find(Hash hash) {
            if (mapValues.empty()) {
                return nullptr;
            }

            // obtain index into backing array
            Hash maskedIndex = hash % mapValues.size();

            unsigned attempts = 0;
            while (attempts < loadCount) {
                if (getHashFromValue(mapValues[maskedIndex]) == hash) {
                    return &mapValues[maskedIndex];
                }
                maskedIndex = (maskedIndex + 1) % mapValues.size();

                attempts++;
            }

            return nullptr;
        }

        /**
         *
         * @tparam PODType -> type of the IntrusiveHashMapPODWrapper<ValueType> that you want to fill
         * @param hash INPUT PARAMETER -> hash that you're searching for
         * @param podForConsumption OUTPUT PARAMETER -> handle to a POD class to be filled
         * @return whether or not the output parameter has been filled with the POD class (which can now be consumed)
         */
        template<typename PODType>
        bool findPODForConsumption(const util::Hash hash, PODType &podForConsumption) {
            if (ValueType *value = find(hash)) {
                podForConsumption = value->get();
                return true;
            }
            return false;
        }

        void erase(util::Hash hash) {
            if (mapValues.empty()) {
                return;
            }

            util::Hash maskedIndex = hash % mapValues.size();

            // TODO - log whether this succeeded optionally? need to figure out debug messages etc
            // OKAY - this doesn't seem to be working; we aren't correctly finding the value to be erased (the loop finishes)
            unsigned spinCount = 0;
            while (spinCount < loadCount) {
                // I think it's because we're setting the hash to 0 somewhere; this means it doesn't find the value to erase
                if (getHashFromValue(mapValues[maskedIndex]) == hash) {
                    valueList.erase(typename IntrusiveList<ValueType>::Iterator(&mapValues[maskedIndex]));
                    static_cast<IntrusiveHashMapEntry<ValueType> &>(mapValues[maskedIndex]).setHash(0);
                    return;
                }
                maskedIndex = (maskedIndex + 1) % mapValues.size();

                spinCount++;
            }
        }

        /**
         * This inserts the hash->value "pair" into the map; it always inserts into the intrusive list even if the value exists.
         * @param hash
         * @param value
         * @return a std::map-style pair with an indicator of whether an insertion happened (should always be true in this case?)
         * and also an iterator to the newly-inserted element in the intrusive hashmap's internal valueList (which is itself an intrusive list)
         */
        typename IntrusiveList<ValueType>::Iterator insertReplace(
            Hash hash, ValueType newValue) {
            if (mapValues.empty()) {
                increaseCapacity();
            }

            // use the hash to compute the index into the backing array
            Hash maskedIndex = hash % mapValues.size();

            unsigned spinCount = 0;
            while (spinCount < loadCount) {
                if (mapValues[maskedIndex].getHash() == 0 || mapValues[maskedIndex].getHash() == hash) {
                    // insert the value, replacing whatever was previously there
                    // (not sure if we need to mess with erasing the previous value from the valueList; I don't think so
                    // since we're storing the values inside the hash map in the mapValues array (ie they're not pointers))
                    mapValues[maskedIndex] = newValue;
                    mapValues[maskedIndex].setHash(hash);
                    auto itr = typename IntrusiveList<ValueType>::Iterator(
                        static_cast<IntrusiveListNode<ValueType> *>(&mapValues[maskedIndex])
                    );
                    valueList.insertFront(itr);
                    return itr;
                }
                maskedIndex = (maskedIndex + 1) % mapValues.size();
                spinCount++;
            }

            // if we failed to insert, the map is not big enough to contain all the elements at the current
            // load level (ie too many collisions), so we have to grow the map and try to reinsert.
            increaseCapacity();
            return insertReplace(hash, newValue);
        }

        /**
         * This inserts the hash->value "pair" into the map; it does not actually insert if the value already exists,
         * which is what is meant by "yield" here.
         * @param hash
         * @param value
         * @return
         */
        std::pair<bool, typename IntrusiveList<ValueType>::Iterator> insertYield(util::Hash hash, ValueType newValue) {
            // need to check if the list is full... not sure how to write a simple boolean expression for that rn tho
            if (mapValues.empty()) {
                increaseCapacity();
            }

            // use the hash to compute the index into the backing array
            Hash maskedIndex = hash % mapValues.size();

            unsigned attempts = 0;
            while (attempts < loadCount) {
                if (getHashFromValue(mapValues[maskedIndex]) == hash) {
                    // the value does exist, so we don't insert to the list or reset the hash; we just return it
                    return std::pair<bool, typename IntrusiveList<ValueType>::Iterator>(
                        false,
                        typename IntrusiveList<ValueType>::Iterator(
                            &mapValues[maskedIndex])
                    );
                }

                if (!getHashFromValue(mapValues[maskedIndex])) {
                    // the value does not exist so we do insert it
                    mapValues[maskedIndex] = newValue;
                    mapValues[maskedIndex].setHash(hash);
                    valueList.insertFront(typename IntrusiveList<ValueType>::Iterator(&mapValues[maskedIndex]));

                    static_cast<IntrusiveHashMapEntry<ValueType>>(mapValues[maskedIndex]).setHash(hash);
                    return std::pair<bool, typename IntrusiveList<ValueType>::Iterator>(
                        true,
                        typename IntrusiveList<ValueType>::Iterator(
                            &mapValues[maskedIndex])
                    );
                }
                maskedIndex = (maskedIndex + 1) % mapValues.size();

                attempts++;
            }


            // if we failed to insert, the map is not big enough to contain all the elements at the current
            // load level (ie too many collisions), so we have to grow the map and try to reinsert.
            increaseCapacity();
            return insertYield(hash, mapValues[maskedIndex]);
        }

        bool empty() {
            return mapValues.empty() && valueList.empty();
        }

        size_t size() {
            return valueList.getSize();
        }

        void clear() {
            valueList.clear();
            mapValues.clear();
            loadCount = 0;
        }

    private:
        static constexpr uint64_t InitialSize = 16;
        static constexpr uint64_t InitialLoadCount = 3;

        /**
         * This should measure the maximum number of hash collisions we've had, so we should only need to search
         * this number times to find a given hashed value
         **/
        unsigned loadCount = 0;

        /**
         * \brief This vector contains the hash map values
         */
        boost::container::stable_vector<ValueType> mapValues = {};

        IntrusiveList<ValueType> valueList = {};

        // TODO - add in a "load count" which I think basically counts the maximum number of collisions you've had so far;
        // this allows you to optimize how many times you increment a masked index when a hash collision occurs

        void increaseCapacity() {
            // make copy of mapValues before resize
            std::vector<ValueType> originalMapValues(mapValues.size());
            std::copy(mapValues.begin(), mapValues.end(), originalMapValues.begin());

            // ensure hash values are set in the original copy list (maybe I should factor this out into its own function)
            if (!mapValues.empty()) {
                size_t mapIndex = 0;
                for (auto itr = originalMapValues.begin(); itr != originalMapValues.end(); ++itr) {
                    if (!reinterpret_cast<IntrusiveHashMapEntry<ValueType> &>(mapValues[mapIndex]).getHash()) {
                        continue;
                    }

                    reinterpret_cast<IntrusiveHashMapEntry<ValueType> &>(*itr).setHash(
                        reinterpret_cast<IntrusiveHashMapEntry<ValueType> &>(mapValues[mapIndex]).getHash());

                    mapIndex++;
                }
            }

            if (mapValues.empty()) {
                mapValues.resize(InitialSize); // simply make room for 1 element if empty then double from there
                loadCount = InitialLoadCount;
            } else {
                loadCount++;
                // resize the stable vector; pointers should remain valid
                mapValues.resize(mapValues.size() * 2); // NOLINT
            }

            // set hash to 0 to signify that the entry is free to be reassigned for each mapValues entry
            std::for_each(mapValues.begin(), mapValues.end(), [&](ValueType &value) {
                reinterpret_cast<IntrusiveHashMapEntry<ValueType> &>(value).setHash(0u);
            });

            // reinsert values into the mapValues array (ie recompute indices for each entry now that size is different)
            valueList.clear();
            size_t mapIndex = 0;
            for (auto itr = originalMapValues.begin(); itr != originalMapValues.end(); ++itr) {
                // inserting value using if-init block
                if (ValueType &value = *itr; !reinsertValue(value)) {
                    // instead of doing the loop thing, maybe I can just re-copy the original mapvalues back
                    // and recurse with the new resized one?
                    std::copy(originalMapValues.begin(), originalMapValues.end(), mapValues.begin());
                    increaseCapacity();
                }
                mapIndex++;
            }
        }

        static util::Hash getHashFromValue(ValueType &value) {
            return reinterpret_cast<IntrusiveHashMapEntry<ValueType> &>(value).getHash();
        }

        bool reinsertValue(ValueType &value) {
            // This is a super simple implementation of this operation, hopefully it's not too basic to work

            util::Hash newHash = getHashFromValue(value);
            auto index = newHash % mapValues.size();

            unsigned spinCount = 0;
            while (spinCount < loadCount) {
                if (!getHashFromValue(mapValues[index])) {
                    mapValues[index].setHash(newHash);
                    mapValues[index] = value;
                    const typename IntrusiveList<ValueType>::Iterator &insertedNodeItr = typename IntrusiveList<
                        ValueType>::Iterator(
                        static_cast<IntrusiveListNode<ValueType> *>(&mapValues[index]));
                    valueList.insertFront(insertedNodeItr);
                    return true;
                }
                index = (index + 1) % mapValues.size();
                spinCount++;
            }
            // in this case we were unable to reinsert the value
            return false;
        }

        bool mapIsFull() {
            return mapValues.size() == valueList.getSize();
        }
    };
} // util
