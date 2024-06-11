//
// Created by paull on 2022-12-28.
//

#pragma once

/**
 * Since we want to use a single buffer if possible, these can be used to represent an individual scene/gir buffer
 * that's been "suballocated" into a larger buffer.
 */
#include <vulkan/vulkan_core.h>
#include "../../../../../utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include <vector>

namespace pEngine::girEngine::backend::vulkan {
    class VulkanBufferSuballocation {
    public:
        struct CreationInput {
            util::UniqueIdentifier subsetUid;
            size_t subsetSize;

            std::vector<unsigned char> subsetDataByteArray;

            size_t globalBufferOffset;
        };

        explicit VulkanBufferSuballocation(const CreationInput &creationInput);

        ~VulkanBufferSuballocation() = default;

        [[nodiscard]] size_t getGlobalBufferOffset() const;

        void setGlobalBufferOffset(size_t offset);

        [[nodiscard]] const util::UniqueIdentifier &getUniqueIdentifier() const;

        [[nodiscard]] size_t getSize() const;

        [[nodiscard]] unsigned char *getData() const;

        // TODO - maybe "rephrase" this signature to be a lil less C-like
        void setData(const unsigned char *newData, size_t newDataSize);

    private:
        util::UniqueIdentifier uniqueIdentifier;
        size_t size;

        size_t globalBufferOffset;

        std::vector<unsigned char> data = {};
    };
} // namespace PGraphics
