//
// Created by paull on 2023-11-26.
//

#pragma once

#include <boost/container_hash/hash.hpp>
#include "../../../../utilities/Hash/HashedObject.hpp"
#include "../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"
#include "VulkanDescriptorSet/DescriptorBinding.hpp"

#include "../../../../lib/murmur3/MurmurHash3.hpp"

// TODO - figure out how best to feed seed values in, this will do for now tho
#define MURMUR_SEED 0x123

namespace pEngine::girEngine::backend::vulkan::descriptor::set {
    using namespace util;

    /**
     * A set of resource handles along with a binding to a particular descriptor slot in a specified set
     */
    struct ResourceDescriptorBindings final : hash::HashedObject {

        unsigned bindingIndex = 0u;

        // TODO - consider packaging these up if that makes things cleaner; I kinda think it's gonna be gross either way
        std::vector<WriteBinding> writeBindings = {};
        std::vector<SourceCopyBinding> copySourceBindings = {};
        std::vector<DestinationCopyBinding> copyDestinationBindings = {};

        ResourceDescriptorBindings() = default;

        ResourceDescriptorBindings(const std::vector<WriteBinding> &writeBindings,
                                   const std::vector<SourceCopyBinding> &copySourceBindings,
                                   const std::vector<DestinationCopyBinding> &copyDestinationBindings)
                : ResourceDescriptorBindings(0u, writeBindings, copySourceBindings, copyDestinationBindings) {

        }

        ResourceDescriptorBindings(unsigned int bindingIndex, const std::vector<WriteBinding> &writeBindings,
                                   const std::vector<SourceCopyBinding> &copySourceBindings,
                                   const std::vector<DestinationCopyBinding> &copyDestinationBindings)
                : bindingIndex(bindingIndex),
                  writeBindings(writeBindings),
                  copySourceBindings(copySourceBindings),
                  copyDestinationBindings(copyDestinationBindings) {}

        Hash computeHash(const Hash seed = 0u) override { // NOLINT
            // it will be tricky to hash the whole struct maybe... I'm still learning the ins and outs of hashing
            // and probably will finish this off with a subpar understanding anyway lol.
            // we could probably get away with hash combining the 3 vector hashes?

            uint64_t tmp[2] = {}; // we have to pass murmur a 128 bit number (ie a size 2 array of 64 bit numbers)

            // TODO - fix these hash types so we don't have problems with non-64-bit hashes

            MurmurHash3_x64_128(&bindingIndex, sizeof(unsigned int), MURMUR_SEED, &tmp);
            Hash bindingIndexHash = tmp[0];

            // compute write bindings array hash
            MurmurHash3_x64_128(writeBindings.data(),
                                writeBindings.size() * sizeof(WriteBinding), //NOLINT
                                MURMUR_SEED, &tmp);
            Hash writeBindingsHash = tmp[0];

            // compute copy bindings array hash
            MurmurHash3_x64_128(copySourceBindings.data(),
                                copySourceBindings.size() * sizeof(SourceCopyBinding), //NOLINT
                                MURMUR_SEED,
                                &tmp);
            Hash copySourceBindingsHash = tmp[0];

            // compute destination bindings array hash
            MurmurHash3_x64_128(copyDestinationBindings.data(),
                                copyDestinationBindings.size() * sizeof(DestinationCopyBinding), //NOLINT
                                MURMUR_SEED, &tmp);
            Hash copyDestBindingsHash = tmp[0];

            // TODO - re-evaluate using boost::hash_combine if it degrades the resulting hashes from Murmur3 too much;
            // if it becomes a big issue we can do something more drastic like combining all binding types into one
            // single monolithic class that we can hash an array of
            hashValue = seed;
            boost::hash_combine(hashValue, bindingIndexHash);
            boost::hash_combine(hashValue, writeBindingsHash);
            boost::hash_combine(hashValue, copySourceBindingsHash);
            boost::hash_combine(hashValue, copyDestBindingsHash);
            return hashValue;
        }

        bool operator==(const ResourceDescriptorBindings &rhs) const {
            return std::tie(static_cast<const hash::HashedObject &>(*this), bindingIndex, writeBindings,
                            copySourceBindings, copyDestinationBindings) ==
                   std::tie(static_cast<const hash::HashedObject &>(rhs), rhs.bindingIndex, rhs.writeBindings,
                            rhs.copySourceBindings, rhs.copyDestinationBindings);
        }

        bool operator!=(const ResourceDescriptorBindings &rhs) const {
            return !(rhs == *this);
        }

        std::vector<VkWriteDescriptorSet> convertWriteBindingsToVkWriteDescriptorSets() {
            // hmm maybe it makes sense to break things down 1 by 1
        }

    };

}

// TODO - write some sort of boost hash function here;
