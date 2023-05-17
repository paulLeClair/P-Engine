//
// Created by paull on 2023-01-15.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <stdexcept>

namespace PGraphics {

    /**
     * I think here we're gonna be doing a similar thing to the VulkanBufferSuballocator, except
     * we're managing descriptor pools and sets for a particular layout.
     *
     * I think descriptor sets are not something we have to synchronize, ie each thread can have a command
     * buffer bind one at any time (since it's really deferring execution to GPU which will handle the actual synch
     * between its own threads)
     *
     * The plan is to have the engine support exactly 4 descriptor sets (the minimum required number by the spec, so it
     * should hypothetically survive even on mobile GPUs), and each descriptor set will have exactly 1 layout,
     * so we should be able to just have a set of these (one for each descriptor set index); this should be easy
     * to make extendable by just having a map from index to allocator, so you can just set a "numberOfDescriptorSetSlots"
     * type thing to use more.
     *
     *
     */
    class VulkanDescriptorSetAllocator {
    public:
        static const std::vector<VkDescriptorPoolSize> DEFAULT_DESCRIPTOR_POOL_SIZES_VECTOR;

        struct CreationInput {
            unsigned int descriptorSetIndex;

            VkDevice device;

            VkDescriptorSetLayout descriptorSetLayout;

            // TODO - evaluate the best way to pass in information about pool dimensions;
            // for now it'll just accept a list upon creation?
            std::vector<VkDescriptorPoolSize> poolSizes;
        };

        explicit VulkanDescriptorSetAllocator(const CreationInput &creationInput) :
                descriptorSetIndex(creationInput.descriptorSetIndex),
                descriptorSetLayout(creationInput.descriptorSetLayout),
                device(creationInput.device),
                descriptorPoolSizes(creationInput.poolSizes) {

            createNewDescriptorPool();

        }

        VkDescriptorSet allocateDescriptorSet();

        [[nodiscard]] const VkDescriptorSetLayout &getDescriptorSetLayout() const {
            return descriptorSetLayout;
        };

        void freeDescriptorPools();

    private:
        unsigned int descriptorSetIndex;

        VkDevice device;

        VkDescriptorSetLayout descriptorSetLayout;

        std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

        std::vector<VkDescriptorPool> descriptorPools = {};
        std::vector<VkDescriptorSet> descriptorSets = {}; // does this work? I think since we're gonna free all descriptor pools each frame, we won't need to care which pool a given set came from

        static const unsigned int MAX_DESCRIPTOR_SETS_PER_POOL = 1000; // instead of a static constant I'll hardcode a default that isn't set anywhere else

        static const unsigned int MAX_DESCRIPTOR_POOLS = 10000; // TODO - determine a good value for this (limit is artificially high for now)

        void createNewDescriptorPool();

        void
        createNewDescriptorPoolAndAttemptAllocationAgain(VkDescriptorSetAllocateInfo &newDescriptorSetAllocateInfo);

        void attemptAllocationAgain(VkDescriptorSetAllocateInfo &newDescriptorSetAllocateInfo);
    };

} // PGraphics
