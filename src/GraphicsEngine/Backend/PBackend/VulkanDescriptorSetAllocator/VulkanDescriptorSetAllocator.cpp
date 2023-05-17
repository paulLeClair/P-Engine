//
// Created by paull on 2023-01-15.
//

#include "VulkanDescriptorSetAllocator.hpp"

#define DEFAULT_DESCRIPTOR_COUNT 200

#define DEFAULT_MAX_NUMBER_OF_DESCRIPTOR_SETS_PER_POOL = 100;

namespace PGraphics {

    const std::vector<VkDescriptorPoolSize> VulkanDescriptorSetAllocator::DEFAULT_DESCRIPTOR_POOL_SIZES_VECTOR = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DEFAULT_DESCRIPTOR_COUNT},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       DEFAULT_DESCRIPTOR_COUNT}
    };

    VkDescriptorSet VulkanDescriptorSetAllocator::allocateDescriptorSet() {
        VkDescriptorSet newDescriptorSetHandle = nullptr;
        descriptorSets.push_back(newDescriptorSetHandle);

        VkDescriptorSetAllocateInfo newDescriptorSetAllocateInfo = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                nullptr,
                descriptorPools.back(),
                1,
                &descriptorSetLayout
        };

        auto allocateDescriptorSetResult = vkAllocateDescriptorSets(device, &newDescriptorSetAllocateInfo,
                                                                    &descriptorSets.back());

        if (allocateDescriptorSetResult == VK_ERROR_OUT_OF_POOL_MEMORY) {
            createNewDescriptorPoolAndAttemptAllocationAgain(newDescriptorSetAllocateInfo);
        } else if (allocateDescriptorSetResult != VK_SUCCESS) {
            // TODO - log!
            throw std::runtime_error(
                    "Error during VulkanDescriptorSetAllocator::allocateDescriptorSet() - descriptor set allocation failed!");
        }

        return descriptorSets.back();
    }

    void VulkanDescriptorSetAllocator::createNewDescriptorPool() {
        if (descriptorPools.size() + 1 > MAX_DESCRIPTOR_POOLS) {
            // TODO - make this configurable (whether it throws or not)
            throw std::runtime_error(
                    "Error during VulkanDescriptorSetAllocator::createNewDescriptorPool() - maximum number of "
                    "pools exceeded!");
        }

        // to initialize, we would just create one descriptor pool and stick it in there
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                nullptr,
                0,
                MAX_DESCRIPTOR_SETS_PER_POOL,
                static_cast<uint32_t>(descriptorPoolSizes.size()),
                descriptorPoolSizes.data()
        };

        VkDescriptorPool newDescriptorPoolHandle = nullptr;
        descriptorPools.push_back(newDescriptorPoolHandle);

        auto createDescriptorPoolResult = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr,
                                                                 &descriptorPools.back());
        if (createDescriptorPoolResult != VK_SUCCESS) {
            // TODO - log!
            throw std::runtime_error(
                    "Error during VulkanDescriptorSetAllocator::createDescriptorPool() - pool creation failed!");
        }
    }

    void VulkanDescriptorSetAllocator::createNewDescriptorPoolAndAttemptAllocationAgain(
            VkDescriptorSetAllocateInfo &newDescriptorSetAllocateInfo) {
        createNewDescriptorPool();
        attemptAllocationAgain(newDescriptorSetAllocateInfo);
    }

    void
    VulkanDescriptorSetAllocator::attemptAllocationAgain(VkDescriptorSetAllocateInfo &newDescriptorSetAllocateInfo) {
        auto allocateDescriptorSetAgainResult = vkAllocateDescriptorSets(device, &newDescriptorSetAllocateInfo,
                                                                         &descriptorSets.back());
        if (allocateDescriptorSetAgainResult != VK_SUCCESS) {
            throw std::runtime_error(
                    "Error during VulkanDescriptorSetAllocator::attemptAllocationAgain "
                    "- descriptor set allocation failed even after creating new descriptor pool!");
        }
    }

    void VulkanDescriptorSetAllocator::freeDescriptorPools() {
        for (auto &descriptorPool: descriptorPools) {
            auto resetDescriptorPoolResult = vkResetDescriptorPool(device, descriptorPool, 0);
            if (resetDescriptorPoolResult != VK_SUCCESS) {
                throw std::runtime_error("Error during VulkanDescriptorSetAllocator::freeDescriptorPools - "
                                         "unable to reset descriptor pool!");
            }
        }
    }


} // PGraphics