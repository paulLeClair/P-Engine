//
// Created by paull on 2023-11-25.
//

#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <random>

#include <boost/container/stable_vector.hpp>

#include "../UniqueIdentifier/UniqueIdentifier.hpp"
#include "../IntrusiveList/IntrusiveList.hpp"
#include "../IntrusiveHashMap/IntrusiveHashMap.hpp"

namespace pEngine::util::objectPool {
    using ObjectPoolIndex = unsigned int;
    using UpdatesSinceLastUsage = unsigned int;

    template<typename ObjectType>
    class ObjectHashPoolEntry : public IntrusiveHashMapEntry<ObjectType> {
    public:
        void setRingIndex(const size_t newIndex) {
            ringIndex = newIndex;
        }

        [[nodiscard]] size_t getRingIndex() const {
            return ringIndex;
        }

        ObjectHashPoolEntry(const ObjectHashPoolEntry &other) = default;

        ObjectHashPoolEntry() = default;

        explicit ObjectHashPoolEntry(const size_t &index) : ringIndex(index) {
        }

    private:
        size_t ringIndex = 0u;
    };
}

namespace pEngine::util::objectPool {
    enum class ReturnedObjectType {
        REQUESTED_OBJECT,
        FRESH_ALLOCATION,
        RECYCLED
    };

    /**
     * For now this is intended to be a super baby-mode simplified version of Themaister's "temporary_hashmap" class
     * from his Granite engine.
     *
     * The idea here will be to imitate the overall behavior of that class but much simpler overall
     *
     */
    template<typename ObjectType, unsigned MaxUpdateCount = 8>
    class ObjectHashPool {
    public:
        struct CreationInput {
            const std::string &name = "";
            UniqueIdentifier uniqueIdentifier = UniqueIdentifier();
        };

        explicit ObjectHashPool(const CreationInput &creationInput)
                : name(creationInput.name),
                  uniqueIdentifier(creationInput.uniqueIdentifier),
                  allocationMap() {
        }

        // TODO - add a function to basically free all the objects and delete all allocations (ie reset the object pool)

        /**
         * This is what you call to trigger recycling of unused objects; it updates the current ring index
         * and erases whatever was in that ring, which contains all elements that were accessed MaxUpdateCount
         */
        void update() {
            currentObjectPoolRingIndex = (currentObjectPoolRingIndex + 1) % MaxUpdateCount;

            // free all existing allocations in the new ring
            IntrusiveList<ObjectAllocation> &currentObjectPoolRing = objectPoolRings[currentObjectPoolRingIndex];
            auto itr = currentObjectPoolRing.begin();
            while (itr != currentObjectPoolRing.end()) {
                auto listNode = itr.get();
                allocationMap.erase(listNode->getHash());
                vacantObjects.push_back(itr);

                itr = ++itr;
            }

            // finally delete all the allocated objects in the newly-current object pool ring
            objectPoolRings[currentObjectPoolRingIndex].clear();
        }

        /**
         * NEW DEVELOPMENT:
         *  I think it is actually valuable to communicate whether the returned object was created new
         *  or recycled from vacant; so we we'll slightly modify how this part works
         *
         * @return a pair with the requested object and a boolean flag which is set to true if the requested object was found
         */
        template<typename... ObjectAllocationParameters>
        std::pair<ObjectType &, ReturnedObjectType>
        requestObject(Hash objectHash, ObjectAllocationParameters &&... objectAllocationParams) {
            // query the map of current allocations for the requested hash
            IntrusiveHashMapPODWrapper<typename IntrusiveList<ObjectAllocation>::Iterator> *queriedValueItr
                    = allocationMap.find(objectHash);

            if (queriedValueItr != nullptr) {
                // in this case, the requested object does exist in the hash map
                ObjectAllocation *queriedValue = queriedValueItr->get().get();

                if (!queriedValue->objectIsAllocated()) {
                    // TODO - error handling / logging!
                }

                // move the existing iterator to the front of the current object pool ring
                if (currentObjectPoolRingIndex != queriedValue->getRingIndex()) {
                    objectPoolRings[currentObjectPoolRingIndex].moveToFront(
                            objectPoolRings[queriedValue->getRingIndex()],
                            queriedValueItr->get()
                    );
                    queriedValue->setRingIndex(currentObjectPoolRingIndex);
                }

                return {
                        *(queriedValue->getObjectPointer()),
                        ReturnedObjectType::REQUESTED_OBJECT
                };
            }
            // else the object does not exist in the hash map;
            // so we obtain a fresh object by either growing the map
            // or reusing a previously-freed entry (which is still classified
            // as a fresh allocation here)
            return obtainFreshObject<ObjectAllocationParameters...>(
                    objectHash,
                    std::forward<ObjectAllocationParameters>(objectAllocationParams)...
            );
        }

    private:
        const unsigned INITIAL_LOAD_COUNT = 3;

        const std::string name;
        const UniqueIdentifier uniqueIdentifier;

        size_t maxSize = 0u;

        /**
         * This will be the new wrapper class for an object allocation? Hopefully it provides some
         * simple-but-janky memory management for this hash pool class and we can finally
         * get the thing working lol.
         */
        class ObjectAllocation : public ObjectHashPoolEntry<ObjectAllocation> {
        public:
            ObjectAllocation() = default;

            ObjectAllocation(const ObjectAllocation &other) {
                allocatedObject.swap(const_cast<std::unique_ptr<ObjectType> &>(other.allocatedObject));
                this->setHash(other.getHash());
                this->setRingIndex(other.getRingIndex());
            }

            /**
             *
             * @tparam ObjectConstructorArgs
             * @param objectConstructorArgs
             * @return
             */
            template<typename... ObjectConstructorArgs>
            bool allocateNewObject(ObjectConstructorArgs &&... objectConstructorArgs) {
                if (allocatedObject) {
                    deleteAllocatedObject();
                }

                allocatedObject = std::make_unique<ObjectType>(
                        std::forward<ObjectConstructorArgs>(objectConstructorArgs)...);
                return allocatedObject == nullptr; // BRO - the fucking unique pointer destructor got called???
            }

            void deleteAllocatedObject() {
                allocatedObject.reset();
            }

            // not sure how to make a getter that returns an object reference since the pointer can be null;
            // in the meantime I'll just have it return the pointer itself
            const ObjectType *getObjectPointer() const {
                return allocatedObject.get();
            }

            ObjectType *getObjectPointer() {
                return allocatedObject.get();
            }

            bool objectIsAllocated() {
                return allocatedObject != nullptr;
            }

            ObjectType &operator->() {
                return *allocatedObject;
            }

            void swapWith(ObjectAllocation &other) {
                allocatedObject.swap(other.allocatedObject);
            }

        private:
            std::unique_ptr<ObjectType> allocatedObject = nullptr;
        };

        /**
         * Each ring holds the set of objects that were accessed in previous update cycles, apart from the current
         * index in which case it contains the set of objects that were accessed in the current update cycle (which
         * includes all objects accessed between the previous update() call and the next update() call that comes in)
         */
        IntrusiveList<ObjectAllocation> objectPoolRings[MaxUpdateCount];

        boost::container::stable_vector<ObjectAllocation> backingAllocations = {};

        /**
         * This controls which of the rings is the active object pool; when objects are accessed they are moved
         * into the current ring, and when an object has not been accessed for MaxUpdateCountBeforeObjectRecycling updates,
         * it will be wiped along with the rest of the objects in the last ring.
         *
         * The ring index is continually updated by incrementing it and then modulo with the number of object pool rings.
         */
        size_t currentObjectPoolRingIndex = 0;

        /**
         * This map stores iterators that each point to unique allocations within the map (which are contained
         * in an intrusive list ring inside @objectPoolRings
         */
        IntrusiveHashMap<IntrusiveHashMapPODWrapper<typename IntrusiveList<ObjectAllocation>::Iterator> >
                allocationMap;

        /**
         * This vector stores iterators to the intrusive list that are not currently holding a valid object
         */
        std::vector<typename IntrusiveList<ObjectAllocation>::Iterator> vacantObjects = {};

        template<typename... P>
        std::pair<ObjectType &, ReturnedObjectType>
        obtainFreshObject(Hash hash, P &&... objectAllocationParameters) {
            typename IntrusiveList<ObjectAllocation>::Iterator nodeItr = typename IntrusiveList<
                    ObjectAllocation>::Iterator();

            bool objectFreshlyAllocated = true;
            if (vacantObjects.empty()) {
                // if we don't have any vacants, allocate a new object and push it to the list
                backingAllocations.push_back(
                        ObjectAllocation()
                );

                objectPoolRings[currentObjectPoolRingIndex].insertFront(
                        typename IntrusiveList<ObjectAllocation>::Iterator(
                                static_cast<IntrusiveListNode<ObjectAllocation> *>(&backingAllocations.back())
                        )
                );
                nodeItr = typename IntrusiveList<ObjectAllocation>::Iterator(
                        objectPoolRings[currentObjectPoolRingIndex].front()
                );
            } else {
                objectFreshlyAllocated = false;
                // else we have vacant nodes, so we just pop one
                nodeItr = vacantObjects.back();
                vacantObjects.pop_back();

                // NOTE: we aren't erasing the vacant object, so it has to be guaranteed to be removed from the intrusive lists upon becoming vacant
                // therefore the intrusive list rings will only contain the allocated object iterators (not empties)
                // if this line of reasoning turns out to be flawed I'll reverse course tho
                objectPoolRings[currentObjectPoolRingIndex].insertFront(nodeItr);
            }

            // update hash and ring index
            nodeItr.get()->setHash(hash);
            nodeItr.get()->setRingIndex(currentObjectPoolRingIndex);
            // allocate the new object using the given parameters
            nodeItr.get()->allocateNewObject(std::forward<P>(objectAllocationParameters)...);

            auto result = allocationMap.insertReplace(
                    hash,
                    IntrusiveHashMapPODWrapper<typename IntrusiveList<ObjectAllocation>::Iterator>( // could something be happening when creating the pod wrapper that's friggin up the allocation?
                            nodeItr
                    )
            );
            // TODO - sanity check the result

            // hmm yeah at this point the node itself says "object not allocated" so something is wiping the allocation
            return {
                    *(nodeItr.get()->getObjectPointer()),
                    objectFreshlyAllocated
                    ? ReturnedObjectType::FRESH_ALLOCATION
                    : ReturnedObjectType::RECYCLED
            };
        }

        /**
         * \brief Maybe unused - not sure though
         * \tparam P
         * \param hash
         * \param objectAllocationParameters
         * \return
         */
        template<typename... P>
        ObjectType &
        reallocateExistingObject(util::Hash hash, P &&... objectAllocationParameters) {
            // we'll just take from the last ring if possible and then keep going until we find
            // a ring that isn't empty and take one from there
            size_t nextRingIndex = (currentObjectPoolRingIndex + 1) % MaxUpdateCount;
            for (unsigned i = 0; i < MaxUpdateCount; i++) {
                if (!objectPoolRings[nextRingIndex].empty()) {
                    // get back() element from this ring, pop it,
                    // and then insert it at the front of the current ring;
                    // this element should be the least-recently-accessed element
                    auto backItr = objectPoolRings[nextRingIndex].rbegin();
                    objectPoolRings[nextRingIndex].popBack();

                    // erase existing hash from allocation map
                    ObjectAllocation &existingObjectAllocation = *backItr.get();
                    const Hash &existingObjectHash = existingObjectAllocation.getHash();
                    allocationMap.erase(existingObjectHash);

                    // reallocate the given object with the new hash
                    existingObjectAllocation.setHash(hash);
                    existingObjectAllocation.setRingIndex(currentObjectPoolRingIndex);
                    existingObjectAllocation.allocateNewObject(std::forward<P>(objectAllocationParameters)...);

                    // add newly-reallocated object back to allocation map and front of current ring
                    objectPoolRings[currentObjectPoolRingIndex].insertFront(backItr);
                    auto insertReplaceResultPair = allocationMap.insertReplace(
                            hash,
                            IntrusiveHashMapPODWrapper<typename IntrusiveList<ObjectAllocation>::Iterator>(backItr)
                    );

                    // return reference to newly-allocated object
                    return *(insertReplaceResultPair.second->get()->getObjectPointer());
                }

                nextRingIndex = (nextRingIndex + 1) % MaxUpdateCount;
            }
            // TODO - log / error handling!
            throw std::runtime_error(
                    "Error in ObjectHashPool::reallocateExistingObject() -> unable to obtain object to overwrite");
        }
    };
} // util
