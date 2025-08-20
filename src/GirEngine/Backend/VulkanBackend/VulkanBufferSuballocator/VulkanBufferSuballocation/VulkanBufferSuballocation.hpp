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
#include <boost/optional.hpp>

namespace pEngine::girEngine::backend::vulkan {
    struct VulkanBufferSuballocation {
        VulkanBufferSuballocation() = default;

        ~VulkanBufferSuballocation() = default;

        util::UniqueIdentifier resourceId = {};
        size_t size = 0;

        size_t globalBufferOffset = 0;

        /**
         * For a given buffer, this is either the geometry binding index (for vertex data) or it will be the
         * descriptor set binding index.
         */
        uint32_t bindingIndex = 0;

        boost::optional<std::vector<unsigned char> > currentData = boost::none;

        VulkanBufferSuballocation(const util::UniqueIdentifier &resource_id, size_t size, size_t global_buffer_offset,
                                  uint32_t binding_index,
                                  const boost::optional<std::vector<unsigned char> > &current_data)
            : resourceId(resource_id),
              size(size),
              globalBufferOffset(global_buffer_offset),
              bindingIndex(binding_index),
              currentData(current_data) {
        }

        VulkanBufferSuballocation(const VulkanBufferSuballocation &other)
            : resourceId(other.resourceId),
              size(other.size),
              globalBufferOffset(other.globalBufferOffset),
              bindingIndex(other.bindingIndex),
              currentData(other.currentData) {
        }

        VulkanBufferSuballocation(VulkanBufferSuballocation &&other) noexcept
            : resourceId(other.resourceId),
              size(other.size),
              globalBufferOffset(other.globalBufferOffset),
              bindingIndex(other.bindingIndex),
              currentData(std::move(other.currentData)) {
        }

        VulkanBufferSuballocation &operator=(const VulkanBufferSuballocation &other) {
            if (this == &other)
                return *this;
            resourceId = other.resourceId;
            size = other.size;
            globalBufferOffset = other.globalBufferOffset;
            bindingIndex = other.bindingIndex;
            currentData = other.currentData.has_value() ? other.currentData : boost::none; // it dies here
            return *this;
        }

        VulkanBufferSuballocation &operator=(VulkanBufferSuballocation &&other) noexcept {
            if (this == &other)
                return *this;
            resourceId = other.resourceId;
            size = other.size;
            globalBufferOffset = other.globalBufferOffset;
            bindingIndex = other.bindingIndex;
            currentData = std::move(other.currentData);
            return *this;
        }
    };
} // namespace PGraphics
