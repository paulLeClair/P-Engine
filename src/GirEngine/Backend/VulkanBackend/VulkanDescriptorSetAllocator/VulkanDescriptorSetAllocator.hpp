//
// Created by paull on 2023-01-15.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <stdexcept>

#include "ResourceDescriptorBindings.hpp"

#include "VulkanDescriptorPool/VulkanDescriptorPool.hpp"

#include "../../../../utilities/ObjectHashPool/ObjectHashPool.hpp"

#include "../VulkanBackend.hpp"


namespace pEngine::girEngine::backend::vulkan::descriptor {
    /**
     *
     * The plan is to have the engine support exactly 4 descriptor sets off the bat (the minimum required number by the spec, so it
     * should hypothetically survive even on mobile GPUs). When it makes sense I'll look into making this customizable.
     *
     */
    class VulkanDescriptorSetAllocator {
    public:
        struct CreationInput {
            const unsigned int descriptorSetIndex = 0;

            VkDevice device = VK_NULL_HANDLE;

            const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};

            const unsigned numberOfWorkerThreads = 0;

            const unsigned initialDescriptorPoolSize = 100u;

            const unsigned maxDescriptorSetsPerPool = 100u;
        };

        void obtainLayoutBindings(const CreationInput &creationInput) {
            layoutBindings.clear();
            layoutBindings.resize(creationInput.descriptorSetLayoutCreateInfo.bindingCount);
            std::memcpy(layoutBindings.data(),
                        creationInput.descriptorSetLayoutCreateInfo.pBindings,
                        creationInput.descriptorSetLayoutCreateInfo.bindingCount * sizeof(VkDescriptorSetLayoutBinding)
            );
        }

        explicit VulkanDescriptorSetAllocator(const CreationInput &creationInput)
            : descriptorSetIndex(creationInput.descriptorSetIndex),
              device(creationInput.device),
              initialDescriptorPoolSize(creationInput.initialDescriptorPoolSize),
              maxDescriptorSetsPerPool(creationInput.maxDescriptorSetsPerPool) {
            obtainLayoutBindings(creationInput);
            createDescriptorSetLayout(creationInput.descriptorSetLayoutCreateInfo);
            initializePerThreadData(creationInput);
        }

        ~VulkanDescriptorSetAllocator() {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        }

        [[nodiscard]] const VkDescriptorSetLayout &getDescriptorSetLayout() const {
            return descriptorSetLayout;
        };

        /**
         * \brief
         * \param threadIndex Which worker thread's data to use in the operation
         * \param bindings Collection of resource handles (represented as UniqueIdentifiers) that we want bound to a descriptor set
         * \return a descriptor set with the required resources bound to it!
         */
        std::pair<VkDescriptorSet, bool> requestDescriptorSet(const unsigned &threadIndex,
                                                              ResourceDescriptorBindings &bindings) {
            bool descriptorSetIsFresh = false;
            if (!bindings.hashValue) {
                bindings.computeHash();
            }
            // hash the given bindings and request a descriptor set handle from the object pool
            const auto [descriptorSet, returnedObjectType]
                    = workerThreadDatas[threadIndex]->descriptorSetAllocations->requestObject(bindings.hashValue);

            // once we obtain the handle, we check if the object was freshly allocated or if it was recycled
            if (returnedObjectType != objectPool::ReturnedObjectType::REQUESTED_OBJECT) {
                // in this case, we were unable to find an existing allocation for that hash,
                // so we assume that we have to potentially reallocate the descriptor set, and definitely re-bind the descriptors for this allocation

                // if we have the specific case where the handle is fresh, we must allocate it from a descriptor pool
                if (returnedObjectType == objectPool::ReturnedObjectType::FRESH_ALLOCATION) {
                    descriptorSetIsFresh = true;
                    // TODO - consider whether we need to make individual descriptor sets freeable if we're going to be potentially reallocating them from different pools;
                    // ideally this won't be an issue and we can just leave it as is, but if needed that can be a hardcoded flag

                    // allocate the handle by giving it to the back() of the thread's descriptor pool vector;
                    // include error checking and enlarge the number of descriptor pools if needed
                    if (workerThreadDatas[threadIndex]->descriptorPools.empty() ||
                        workerThreadDatas[threadIndex]->descriptorPools.back()->isFull()) {
                        // push new descriptor pool to back of vector
                        workerThreadDatas[threadIndex]->descriptorPools.emplace_back(
                            std::make_unique<VulkanDescriptorPool>(
                                VulkanDescriptorPool::CreationInput{
                                    UniqueIdentifier(),
                                    "thread" + std::to_string(threadIndex) + "DescriptorPool" +
                                    std::to_string(workerThreadDatas[threadIndex]->descriptorPools.size()),
                                    descriptorSetLayout,
                                    layoutBindings,
                                    device,
                                    initialDescriptorPoolSize,
                                    maxDescriptorSetsPerPool
                                })
                        );
                    }
                    VkDescriptorPool descriptorPool
                            = workerThreadDatas[threadIndex]->descriptorPools.back()->getHandle();
                    // TODO -> avoid these java-like wrapper classes and just allocate the descriptor set here
                    VkDescriptorSetAllocateInfo allocationInfo = {
                        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                        nullptr,
                        descriptorPool,
                        1,
                        &descriptorSetLayout
                    };
                    const auto successfullyAllocated =
                            vkAllocateDescriptorSets(device, &allocationInfo, &descriptorSet) == VK_SUCCESS;
                    if (!successfullyAllocated) {
                        // TODO -> log! don't throw!
                        throw std::runtime_error(
                            "Error in VulkanDescriptorSetAllocator::requestDescriptorSet -> failed to allocate descriptor sets");
                    }
                }

                // in either case we have to re-bind the resources that the user provided if the returned object was not
                // the requested one, which of course is done through "updating" in vulkan
                // TODO -> update the set directly here
                // hmm.... one issue: we are going to need access to the resource repository
            }
            return {descriptorSet, descriptorSetIsFresh};
        }

        [[nodiscard]] unsigned int getDescriptorSetIndex() const {
            return descriptorSetIndex;
        }

    private:
        const unsigned int descriptorSetIndex = 0;

        VkDevice device = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = {};

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};

        unsigned initialDescriptorPoolSize = 100u;

        unsigned maxDescriptorSetsPerPool = 100u;

        /**
         * \brief This contains one particular worker thread's descriptor pools & sets;
         *
         */
        struct PerThreadData {
            /**
             * \brief This should hold all the descriptor sets we've allocated.
             *
             * For the single-animated-model demo I'm going to try and streamline out some of these
             * abstractions that don't seem super necessary (I'm hoping I can just get away with
             * using a raw VkDescriptorSet instead of the wrapper class)
             */
            std::unique_ptr<objectPool::ObjectHashPool<VkDescriptorSet> > descriptorSetAllocations;

            // each thread will also have to maintain a set of descriptor pools to be expanded on demand
            std::vector<std::unique_ptr<descriptor::VulkanDescriptorPool> > descriptorPools = {};
        };

        std::vector<std::unique_ptr<PerThreadData> > workerThreadDatas = {};

        //NOLINTNEXTLINE
        void createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &descriptorSetLayoutCreateInfo) {
            // TODO - better error handling here
            if (device == VK_NULL_HANDLE) {
                throw std::runtime_error(
                    "Error in VulkanDescriptorSetAllocator::createDescriptorSetLayout - device is null!");
            }
            auto result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr,
                                                      &descriptorSetLayout);
            if (result != VK_SUCCESS) {
                // TODO - error handling here
                throw std::runtime_error(
                    "Error in VulkanDescriptorSetAllocator::createDescriptorSetLayout - layout creation failed!");
            }
        }

        void initializePerThreadData(const CreationInput &input) {
            workerThreadDatas.resize(input.numberOfWorkerThreads);
            for (auto &perThreadData: workerThreadDatas) {
                perThreadData = std::make_unique<PerThreadData>();

                // initialize this thread's data
                perThreadData->descriptorSetAllocations
                        = std::make_unique<objectPool::ObjectHashPool<VkDescriptorSet> >(
                            objectPool::ObjectHashPool<VkDescriptorSet>::CreationInput{}
                        );

                // add initial descriptor pool
            }
        }

        /**
         * Hmm. I imagine we can just convert our backend structs to the appropriate Vulkan ones,
         * but maybe this is a bad idea I'm not sure.
         *
         * One issue is that we need a particular descriptor set for each vkwritedescriptorset ...
         *
         * Right.. so I think what we can do maybe is just have this take in a particular descriptor set allocator
         * so that it can allocate descriptor sets on the fly for it.
         *
         * But we also have to remember that we're differentiating the hash-tabled descriptor sets by their particular
         * bound resources (which I think is actually just hashing their UIDs? yet another reason for extra UID sanity checking),
         * luckily I think that's also part of this ResourceDescriptorBindings struct.
         *
         * The whole reason we want these is to supply them to a command buffer, particularly the "update descriptor sets"
         * command afaik.
         *
         */
        static std::vector<VkWriteDescriptorSet>
        convertWriteBindingsToVkWriteDescriptorSets(const std::vector<WriteBinding> &writeBindings) {
            // ISSUE: for some reason I can't include VulkanDescriptorSetAllocator here
            std::vector<VkWriteDescriptorSet> writeDescriptorSets(writeBindings.size());
            uint32_t writeDescriptorIndex = 0;
            // okay this might not be too hard actually
            for (auto &writeBinding: writeBindings) {
                writeDescriptorSets[writeDescriptorIndex] = VkWriteDescriptorSet{
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    nullptr,

                };
                writeDescriptorIndex++;
            }
            return writeDescriptorSets;
        }
    };
} // PGraphics
