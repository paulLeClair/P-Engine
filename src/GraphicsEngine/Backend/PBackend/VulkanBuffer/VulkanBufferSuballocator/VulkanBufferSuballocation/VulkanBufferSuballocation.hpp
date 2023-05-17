//
// Created by paull on 2022-12-28.
//

#pragma once

/**
 * Since we want to use a single buffer if possible, I think it makes sense to structure things so that
 * each "Buffer" in the Scene becomes a VulkanBufferSuballocation,
 */
#include <vulkan/vulkan_core.h>
#include "../../../../../../EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp"

#include <vector>

namespace PGraphics {

    class VulkanBufferSuballocation {
    public:
        struct CreationInput {
            PUtilities::UniqueIdentifier subsetUid;
            unsigned long long subsetSize;

            std::vector<unsigned char> subsetDataByteArray;

            unsigned long long globalBufferOffset;
        };

        explicit VulkanBufferSuballocation(const CreationInput &creationInput);

        ~VulkanBufferSuballocation() = default;

        [[nodiscard]] unsigned long long int getGlobalBufferOffset() const;

        void setGlobalBufferOffset(unsigned long long int offset);

        [[nodiscard]] const PUtilities::UniqueIdentifier &getUniqueIdentifier() const;

        [[nodiscard]] unsigned long long int getSize() const;

        [[nodiscard]] void *getData() const;

        void setData(unsigned int *newData);

    private:
        PUtilities::UniqueIdentifier uniqueIdentifier;
        unsigned long long size;

        unsigned long long globalBufferOffset;

        std::vector<unsigned char> data = {};
    };

} // namespace PGraphics