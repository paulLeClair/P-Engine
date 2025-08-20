//
// Created by paull on 2023-12-10.
//

#pragma once

using namespace pEngine::util;

namespace pEngine::girEngine::backend::vulkan::descriptor {
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


        [[nodiscard]] bool isFull() const {
            return descriptorSets.size() == maxNumberOfSets;
        }

        void resetDescriptorPool(const VkDescriptorPoolResetFlags flags = 0) {
            vkResetDescriptorPool(device, descriptorPool, flags);
            descriptorSets.clear();
        }

        [[nodiscard]] size_t getNumberOfAllocatedDescriptors() const {
            return descriptorSets.size();
        }

        [[nodiscard]] const std::vector<VkDescriptorSet> &getDescriptorSets() const {
            return descriptorSets;
        }

        [[nodiscard]] VkDescriptorPool getHandle() const {
            return descriptorPool;
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

        std::vector<VkDescriptorSet> descriptorSets;

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
