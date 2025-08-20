//
// Created by paull on 2024-10-27.
//

#pragma once

namespace pEngine::girEngine::backend::vulkan {
    /**
     * This type is meant to act as an external handle to a particular
     * "suballocated" portion of a @VulkanBuffer, as part of some instance of
     * the @VulkanBufferSuballocator type.
     *
     *
     */
    struct VulkanBufferSuballocationHandle {
        /**
         * These should correspond to an existing buffer suballocation that lives internally to a
         * suballocator object, and contains the UID that we use to identify it with the scene resource
         * that it originally came from.
         */
        VulkanBufferSuballocation suballocation;

        /**
         * This should be the parent buffer, inside of which your suballocation lives.
         * It should be used to actually make use of the suballocation externally (I'm hoping).
         */
        VkBuffer parentBuffer;

        VulkanBufferSuballocationHandle() = default;

        VulkanBufferSuballocationHandle(const VulkanBufferSuballocation &suballocation, const VkBuffer parent_buffer)
            : suballocation(suballocation),
              parentBuffer(parent_buffer) {
        }

        VulkanBufferSuballocationHandle(const VulkanBufferSuballocationHandle &other) = default;

        VulkanBufferSuballocationHandle(VulkanBufferSuballocationHandle &&other) noexcept = default;

        // this default ctor doesn't seem to work nicely
        VulkanBufferSuballocationHandle &operator
        =(const VulkanBufferSuballocationHandle &other) {
            suballocation = other.suballocation;
            parentBuffer = other.parentBuffer;
            return *this;
        }

        VulkanBufferSuballocationHandle &operator=(VulkanBufferSuballocationHandle &&other) noexcept = default;
    };
}
