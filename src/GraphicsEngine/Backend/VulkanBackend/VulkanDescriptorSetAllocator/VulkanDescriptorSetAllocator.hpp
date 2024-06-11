//
// Created by paull on 2023-01-15.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <stdexcept>

#include "ResourceDescriptorBindings.hpp"

#include "VulkanDescriptorSet/VulkanDescriptorSet.hpp"
#include "VulkanDescriptorPool/VulkanDescriptorPool.hpp"

#include "..\..\..\..\utilities\ObjectHashPool\ObjectHashPool.hpp"

#include "../VulkanBackend.hpp"


namespace pEngine::girEngine::backend::vulkan::descriptor {
    /**
     * Coming back to this now that we have the object pool made...
     *
     *
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
        VkDescriptorSet &requestDescriptorSet(const unsigned &threadIndex,
                                              set::ResourceDescriptorBindings &bindings) {
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
                    // TODO - consider whether we need to make individual descriptor sets freeable if we're going to be potentially reallocating them from different pools;
                    // ideally this won't be an issue and we can just leave it as is, but if needed that can be a hardcoded flag

                    // allocate the handle by giving it to the back() of the thread's descriptor pool vector;
                    // include error checking and enlarge the number of descriptor pools if needed
                    if (workerThreadDatas[threadIndex]->descriptorPools.empty() || workerThreadDatas[threadIndex]->descriptorPools.back()->isFull()) {
                        // push new descriptor pool to back of vector
                        workerThreadDatas[threadIndex]->descriptorPools.emplace_back(
                                std::make_unique<pool::VulkanDescriptorPool>(
                                        pool::VulkanDescriptorPool::CreationInput{
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
                    auto &descriptorPool = *workerThreadDatas[threadIndex]->descriptorPools.back();
                    descriptorPool.allocateDescriptorSet(descriptorSet);
                }

                // in either case we have to re-bind the resources that the user provided if the returned object was not
                // the requested one, which of course is done through "updating" in vulkan
                 descriptorSet.updateSet(device, bindings);

            }
            return descriptorSet.getVkDescriptorSetHandle();
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
             */
            std::unique_ptr<objectPool::ObjectHashPool<set::VulkanDescriptorSet> > descriptorSetAllocations;
//            std::unique_ptr<objectPool::ObjectHashPool<VkDescriptorSet>> descriptorSetAllocations;

            // each thread will also have to maintain a set of descriptor pools to be expanded on demand
            std::vector<std::unique_ptr<pool::VulkanDescriptorPool>> descriptorPools = {};
        };

        std::vector<std::unique_ptr<PerThreadData> > workerThreadDatas = {};

        //NOLINTNEXTLINE
        void createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &descriptorSetLayoutCreateInfo) {
            // TODO - better error handling here
            if (device == VK_NULL_HANDLE) {
                throw std::runtime_error(
                    "Error in VulkanDescriptorSetAllocator::createDescriptorSetLayout - device is null!");
            }
            if (vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr,
                                            &descriptorSetLayout) != VK_SUCCESS) {
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
                    = std::make_unique<util::objectPool::ObjectHashPool<set::VulkanDescriptorSet>>(
                            util::objectPool::ObjectHashPool<set::VulkanDescriptorSet>::CreationInput{}
                            );

                // add initial descriptor pool

            }
        }
    };
} // PGraphics
