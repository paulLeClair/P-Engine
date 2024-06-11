//
// Created by paull on 2023-12-10.
//

#pragma once

#include "../VulkanDescriptorSet/VulkanDescriptorSet.hpp"

namespace pEngine::girEngine::backend::vulkan::descriptor::pool {
    /**
     * \brief This should wrap a descriptor pool, that's pretty much it
     */
    class VulkanDescriptorPool {
    public:
        static constexpr uint32_t DEFAULT_POOL_SIZE = 16;

        struct CreationInput {
            UniqueIdentifier uid;

            std::string label;

            VkDescriptorSetLayout layout;

            const std::vector<VkDescriptorSetLayoutBinding> bindings; // note: we may want to store this too

            VkDevice device = VK_NULL_HANDLE;

            uint32_t poolSize = DEFAULT_POOL_SIZE;

            uint32_t maxNumberOfSets;

            bool allowDescriptorSetsToBeFreed = false;
            bool enableUpdateAfterBind = false;
            bool disableInternalAllocation = true;
        };

        explicit VulkanDescriptorPool(const CreationInput &creationInput)
                : uid(creationInput.uid),
                  label(creationInput.label),
                  disableInternalAllocation(creationInput.disableInternalAllocation),
                  layout(creationInput.layout),
                  device(creationInput.device),
                  poolSize(creationInput.poolSize),
                  maxNumberOfSets(creationInput.maxNumberOfSets),
                  allowDescriptorSetsToBeFreed(creationInput.allowDescriptorSetsToBeFreed),
                  enableUpdateAfterBind(creationInput.enableUpdateAfterBind) {

            VkFlags poolCreateFlags = (allowDescriptorSetsToBeFreed)
                                      ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
                                      : 0;
            poolCreateFlags &= (enableUpdateAfterBind) // is this binary logic correct? B)
                               ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
                               : 1;

            std::vector<VkDescriptorPoolSize> descriptorPoolSizes = getDescriptorPoolSizesFromLayoutBindings(
                    creationInput.bindings);
            const VkDescriptorPoolCreateInfo poolCreateInfo = {
                    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    nullptr,
                    poolCreateFlags,
                    maxNumberOfSets,
                    static_cast<uint32_t>(descriptorPoolSizes.size()),
                    descriptorPoolSizes.data()
            };
            if (const auto createPoolResult = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool);
                    createPoolResult != VK_SUCCESS) {
                throw std::runtime_error("Error in VulkanDescriptorPool() -> unable to create VkDescriptorPool!");
            }
        }

        ~VulkanDescriptorPool() {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        }

        [[nodiscard]] VkDescriptorPool getHandle() const {
            return descriptorPool;
        }

        [[nodiscard]] VkDescriptorSetLayout getLayout() const {
            return layout;
        }

        [[nodiscard]] VkDevice getDevice() const {
            return device;
        }

        [[nodiscard]] bool isFull() const {
            return descriptorSets.size() == maxNumberOfSets;
        }

        /**
         * \brief This is the *external* version of the descriptor set allocation code, which is meant to be used
         * with an externally-created descriptor set handle that is then handed into the pool to be given an allocation.
         * If the pool has individual descriptor set freeing enabled, then the user must be careful to hold on to the
         * descriptor set handles they use because they aren't tracked internally here.
         *
         * \param descriptorSet -> the handle to the descriptor set that will point to the new allocation
         * \return true if the allocation succeeded with the given handle; false if the pool is full (or the allocation failed?)
         */
        bool allocateDescriptorSet(set::VulkanDescriptorSet &descriptorSet) {
            if (isFull()) {
                return false;
            }

            // Okay, so here we need to be directly allocating the set using the pool, unlike the old way
            // where we let the descriptor set allocate itself (using the pool)
            VkDescriptorSetAllocateInfo allocationInfo = {
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    nullptr,
                    descriptorPool,
                    1,
                    &layout
            };
            bool allocationWasSuccessful =
                    vkAllocateDescriptorSets(device, &allocationInfo, &descriptorSet.getVkDescriptorSetHandle()) ==
                    VK_SUCCESS;
            descriptorSet.setAllocationFlag(allocationWasSuccessful);
            return allocationWasSuccessful;
        }

        /**
         * \brief Allocates a new descriptor set **INTERNALLY** to this pool and returns a pointer to it, provided the pool is not full.
         * Note: using this will give you the handle to the descriptor set, but it will exist inside the pool object that created it.
         *
         * \return nullptr if descriptor pool has already allocated the maximum number of descriptor sets, or a pointer to the allocated set,
         * or if internal allocation is disabled via @disableInternalAllocation
         */
        [[nodiscard]] const set::VulkanDescriptorSet *internallyAllocateDescriptorSet() {
            if (disableInternalAllocation || isFull()) {
                return nullptr;
            }

            const auto createInput = set::VulkanDescriptorSet::CreationInput{
                    UniqueIdentifier(),
                    label + "PoolAllocation" + std::to_string(descriptorSets.size() - 1), // TODO - better labeling
                    device,
                    descriptorPool,
                    layout
            };
            descriptorSets.push_back(
                    set::VulkanDescriptorSet(set::VulkanDescriptorSet(createInput))
            );
            return &descriptorSets.back();
        }

        void resetDescriptorPool(const VkDescriptorPoolResetFlags flags = 0) {
            vkResetDescriptorPool(device, descriptorPool, flags);
            descriptorSets.clear();

            // if we end up resetting often, it may make more sense to "reuse" the descriptor sets
            // and allow the descriptor sets to be reallocated (instead of allocating in the ctor like it does now)
        }

        [[nodiscard]] size_t getNumberOfAllocatedDescriptors() const {
            return descriptorSets.size();
        }

        [[nodiscard]] const std::vector<set::VulkanDescriptorSet> &getDescriptorSets() const {
            return descriptorSets;
        }

    private:
        UniqueIdentifier uid;

        std::string label;

        bool disableInternalAllocation = true;

        VkDescriptorSetLayout layout;

        VkDevice device;

        uint32_t poolSize;

        uint32_t maxNumberOfSets;

        bool allowDescriptorSetsToBeFreed = false;
        bool enableUpdateAfterBind = false;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        std::vector<set::VulkanDescriptorSet> descriptorSets;

        static std::vector<VkDescriptorPoolSize> getDescriptorPoolSizesFromLayoutBindings(
                const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
            // basically we should just only add the descriptor pool sizes that we need
            std::vector<VkDescriptorPoolSize> sizes = {};

            for (const auto &binding: bindings) {
                VkDescriptorPoolSize newBindingPoolSize = {
                        binding.descriptorType,
                        binding.descriptorCount
                };
                sizes.push_back(newBindingPoolSize);
            }

            return sizes;
        }
    };
} // descriptor
