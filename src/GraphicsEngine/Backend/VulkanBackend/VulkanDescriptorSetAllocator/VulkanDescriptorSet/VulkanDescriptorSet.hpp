//
// Created by paull on 2023-12-10.
//

#pragma once

#include "DescriptorBinding.hpp"

#include <memory>
#include <utility>

namespace pEngine::girEngine::backend::vulkan::descriptor::set {
    class VulkanDescriptorSet {
    public:
        struct CreationInput {
            UniqueIdentifier uid;
            std::string label;

            VkDevice device;

            VkDescriptorPool descriptorPool;

            VkDescriptorSetLayout layout;
        };

        explicit VulkanDescriptorSet(const CreationInput &input) : uid(input.uid), label(input.label) {
            // allocate using pool that was given
            if (!allocateDescriptorSet(input.device, input.descriptorPool, input.layout)) {
                // TODO - replace this with logging/actual error handling
                throw std::runtime_error("Error in VulkanDescriptorSet() -> Unable to allocate set!");
            }
            hasBeenAllocated = true;
        }

        VulkanDescriptorSet() : uid(UniqueIdentifier()) {
            // the default constructor should basically *not* allocate the set, so it's just a free-hanging handle
        }

        VulkanDescriptorSet(const UniqueIdentifier uid, std::string label) : uid(uid), label(std::move(label)) {
        }

        [[nodiscard]] VkDescriptorSet &getVkDescriptorSetHandle() {
            return descriptorSet;
        }

        /**
         * \brief
         * \param device
         * \param pool
         * \param layout
         * \return true if the descriptor set was allocated successfully using the given parameters, else false if the pool is full or parameters are invalid.
         */
        bool allocateDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout) {
            if (!device || !pool || !layout) {
                return false;
            }

            const VkDescriptorSetAllocateInfo allocInfo = {
                    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    nullptr,
                    pool,
                    1,
                    &layout
            };

            const auto result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
            hasBeenAllocated = (result == VK_SUCCESS);
            return hasBeenAllocated;
        }

        /**
         * This is how you actually bind resources to a particular descriptor set; see the DescriptorBinding
         * subclasses for more info
         *
         * TODO - change function signature to not use shared pointer - instead pass 3 vectors in
         */
        void updateSet(const VkDevice &device,
                       const ResourceDescriptorBindings &newBindings) {
            // idea: build up struct to call vkUpdateDescriptorSets() with info gleaned from the newBindings
            std::vector<VkWriteDescriptorSet> descriptorSetWrites = {};
            std::vector<VkCopyDescriptorSet> descriptorSetCopies = {};

            descriptorSetWrites.reserve(newBindings.writeBindings.size());
            for (const auto &writeBinding: newBindings.writeBindings) {
                descriptorSetWrites.push_back({
                                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                      nullptr,
                                                      descriptorSet,
                                                      writeBinding.bindingIndex,
                                                      writeBinding.descriptorArrayIndex,
                                                      writeBinding.descriptorCount,
                                                      writeBinding.descriptorType,
                                                      &writeBinding.boundImage.get_value_or({}),
                                                      &writeBinding.boundBuffer.get_value_or({}),
                                                      &writeBinding.boundTexelBufferView.get_value_or({})
                                              });

            }
            descriptorSetCopies.reserve(
                    newBindings.copySourceBindings.size() + newBindings.copyDestinationBindings.size());
            for (const auto &copySourceBinding: newBindings.copySourceBindings) {
                descriptorSetCopies.push_back({
                                                      VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
                                                      nullptr,
                                                      descriptorSet,
                                                      copySourceBinding.sourceBindingIndex,
                                                      copySourceBinding.sourceArrayElement,
                                                      copySourceBinding.destinationDescriptorSet,
                                                      copySourceBinding.destinationBindingIndex,
                                                      copySourceBinding.destinationArrayElement,
                                                      copySourceBinding.descriptorCount
                                              });

            }
            for (auto &destinationCopyBinding: newBindings.copyDestinationBindings) {
                descriptorSetCopies.push_back({
                                                      VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
                                                      nullptr,
                                                      destinationCopyBinding.sourceDescriptorSet,
                                                      destinationCopyBinding.sourceBindingIndex,
                                                      destinationCopyBinding.sourceArrayElement,
                                                      descriptorSet,
                                                      destinationCopyBinding.destinationBindingIndex,
                                                      destinationCopyBinding.destinationArrayElement,
                                                      destinationCopyBinding.descriptorCount
                                              });

            }
            // TODO - support case where we specify the copy dest and source at the same time

            // use the api call to actually update the descriptors!
            vkUpdateDescriptorSets(
                    device,
                    descriptorSetWrites.size(),
                    descriptorSetWrites.data(),
                    descriptorSetCopies.size(),
                    descriptorSetCopies.data()
            );
        }

        [[nodiscard]] bool hasDescriptorSetBeenAllocated() const {
            return hasBeenAllocated;
        }

        void setAllocationFlag(bool newValue) {
            // TODO - sanity checking against the current state of the set (this shouldn't be used for much)
            hasBeenAllocated = newValue;
        }

    private:
        UniqueIdentifier uid;
        std::string label;

        bool hasBeenAllocated = false;

        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

        // descriptor set layout bindings are probably required
        // not sure actually now that I'm returning to this. wouldn't be hard to add tho

        // one thing: do we want to tie a descriptor set object to a particular layout? might be good to prevent fuckery
    };
} // descriptor
