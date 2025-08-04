//
// Created by paull on 2024-02-02.
//

#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"
#include "../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp"

using namespace pEngine::girEngine::backend::vulkan;
using namespace pEngine::girEngine::backend::appContext::vulkan;
using namespace pEngine::girEngine::backend::vulkan::descriptor;

class DescriptorSetAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        constexpr uint32_t TEST_VULKAN_API_VERSION = VK_MAKE_API_VERSION(0, 1, 3, 261);
        constexpr uint32_t TEST_APP_ENGINE_VERSION = VK_MAKE_VERSION(0, 1, 0);

        testInstance = std::make_shared<VulkanInstance>(VulkanInstance::CreationInput{
                {
                        VulkanInstance::SupportedInstanceExtension::SURFACE_EXTENSION
                },
                {VulkanInstance::SupportedLayers::VALIDATION_LAYER},
                "testEngineName",
                "testAppName",
                TEST_APP_ENGINE_VERSION,
                TEST_APP_ENGINE_VERSION,
                TEST_VULKAN_API_VERSION
        });
        testPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(VulkanPhysicalDevice::CreationInput{
                UniqueIdentifier(),
                "testPhysicalDevice",
                testInstance->getVkInstance(),
                PhysicalDeviceChoice::BEST_GPU,
                VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                TEST_VULKAN_API_VERSION
        });
        testDevice = std::make_shared<VulkanLogicalDevice>(VulkanLogicalDevice::CreationInput{
                "testDevice",
                testPhysicalDevice->getPhysicalDevice()
        });

        VmaAllocatorCreateInfo allocatorCreateInfo = {
                0,
                testPhysicalDevice->getPhysicalDevice(),
                testDevice->getVkDevice(),
                0,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                testInstance->getVkInstance(),
                TEST_VULKAN_API_VERSION
        };

        if (vmaCreateAllocator(&allocatorCreateInfo, &testVmaAllocator) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create VmaAllocator!");
        }
    }

    void TearDown() override {
    }

    std::shared_ptr<VulkanInstance> testInstance;
    std::shared_ptr<VulkanPhysicalDevice> testPhysicalDevice;
    std::shared_ptr<VulkanLogicalDevice> testDevice;
    VmaAllocator testVmaAllocator;
};

// TODO -> modify these tests and make sure everything's working as expected

//TEST_F(DescriptorSetAllocatorTest, TestDescriptorBindingHashes) {
//    // Here we probably want to test out that the current descriptor set binding hash computation
//    // method isn't too crazy... if it becomes very expensive for lots of bindings then I'll have to
//    // throw it all out
//
//    const boost::optional<VkDescriptorBufferInfo> TEST_BUFFER_INFO = {
//        VkDescriptorBufferInfo{
//            VK_NULL_HANDLE,
//            0,
//            100u
//        }
//    };
//
//    const boost::optional<VkDescriptorImageInfo> TEST_IMAGE_INFO = {
//        VkDescriptorImageInfo{
//            VK_NULL_HANDLE,
//            VK_NULL_HANDLE,
//            VK_IMAGE_LAYOUT_GENERAL
//        }
//    };
//
//    const auto TEST_BINDINGS_DATA = descriptor::set::ResourceDescriptorBindings(
//        {
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                0,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                boost::none,
//                boost::optional<VkDescriptorBufferInfo>(TEST_BUFFER_INFO),
//                boost::none
//            },
//            descriptor::set::WriteBinding{
//                UniqueIdentifier(),
//                1,
//                0,
//                1,
//                VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
//                boost::optional<VkDescriptorImageInfo>(TEST_IMAGE_INFO),
//                boost::none,
//                boost::none
//            }
//        },
//        {
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            },
//            descriptor::set::SourceCopyBinding{
//                UniqueIdentifier(),
//                1,
//                4,
//                0,
//                3,
//                0,
//                VK_NULL_HANDLE
//            }
//        },
//        {}
//    );
//
//    auto testBindings = TEST_BINDINGS_DATA;
//
//    auto beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
//    testBindings.computeHash();
//    Hash hash = testBindings.hashValue;
//
//    auto afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
//    auto timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
//
//    std::cout << "Descriptor set hash: " << std::to_string(hash) << std::endl;
//
//    std::cout << "Time for single descriptor set hash: "
//            << std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(timeElapsed).count())
//            << "ns" << std::endl;
//
//    // make sure that when we hash two identical objects they get the same hash
//    descriptor::set::ResourceDescriptorBindings sameBindings = TEST_BINDINGS_DATA;
//    sameBindings.computeHash();
//    ASSERT_EQ(hash, sameBindings.hashValue);
//
//    beforeRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
//    ASSERT_TRUE(sameBindings == testBindings);
//    afterRequestLoopTimeStamp = std::chrono::high_resolution_clock::now();
//    timeElapsed = afterRequestLoopTimeStamp - beforeRequestLoopTimeStamp;
//    std::cout << "Time to compare two ResourceDescriptorBinding objects: " << std::to_string(
//                std::chrono::duration_cast<std::chrono::nanoseconds>(timeElapsed).count())
//            << "ns" << std::endl;
//
//    // assert that changing an existing ResourceBinding's hash changes appropriately when we change a value
//    auto inputAttachmentWriteBindingsSizeBefore = sameBindings.writeBindings.size();
//    sameBindings.writeBindings.emplace_back(descriptor::set::WriteBinding{
//            UniqueIdentifier(),
//            0,
//            0,
//            1,
//            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
//            TEST_IMAGE_INFO,
//            {},
//            {}
//        }
//    );
//
//
//    ASSERT_TRUE(sameBindings.writeBindings.size() == inputAttachmentWriteBindingsSizeBefore + 1);
//    ASSERT_TRUE(
//        sameBindings.writeBindings.size() - testBindings.writeBindings.size() == 1);
//    ASSERT_TRUE(sameBindings.writeBindings != testBindings.writeBindings);
//    ASSERT_TRUE(sameBindings != testBindings);
//    sameBindings.computeHash(); // recompute hash
//    ASSERT_NE(hash, sameBindings.hashValue);
//
//    // recomputing unchanged object's hash should not change the hash
//    testBindings.computeHash();
//    ASSERT_EQ(testBindings.hashValue, hash);
//
//    // verify that changing a sub-element inside a binding struct will result in a different hash
//    testBindings.writeBindings[0].bindingIndex++;
//    testBindings.computeHash();
//    ASSERT_NE(testBindings.hashValue, hash);
//}
//
//TEST_F(DescriptorSetAllocatorTest, TestDescriptorPools_SetCreationWithInternalAllocation) {
//    constexpr uint32_t TEST_POOL_SIZE = 10;
//
//    const std::vector<VkDescriptorSetLayoutBinding> TEST_LAYOUT_BINDINGS = {
//        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr},
//        {
//            1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
//            nullptr
//        },
//        {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
//        {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 3, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr}
//    };
//
//    const VkDescriptorSetLayoutCreateInfo TEST_LAYOUT_CREATE_INFO = {
//        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//        nullptr,
//        0,
//        // using no flags for testing purposes? might want to have tests that make use of certain flags as relevant
//        static_cast<uint32_t>(TEST_LAYOUT_BINDINGS.size()),
//        TEST_LAYOUT_BINDINGS.data()
//    };
//
//    // create test layout to use
//    VkDescriptorSetLayout testLayout = VK_NULL_HANDLE;
//    ASSERT_EQ(
//        vkCreateDescriptorSetLayout(testDevice->getVkDevice(), &TEST_LAYOUT_CREATE_INFO, nullptr, &testLayout),
//        VK_SUCCESS);
//
//    auto testPool = descriptor::pool::VulkanDescriptorPool(
//        descriptor::pool::VulkanDescriptorPool::CreationInput{
//            UniqueIdentifier(),
//            "testPool",
//            testLayout,
//            TEST_LAYOUT_BINDINGS,
//            testDevice->getVkDevice(),
//            TEST_POOL_SIZE,
//            TEST_POOL_SIZE, // arbitrary max
//            false, // allow freeing descriptor sets -> false
//            false, // allow updating after bind -> false (TODO - find better name for this flag)
//            false // disable internal allocation -> false
//        }
//    );
//
//    // okay so the pool is being created! now we can write & test the stuff for requesting descriptor sets from
//    // each descriptor pool.
//    for (int i = 0; i < TEST_POOL_SIZE; i++) {
////        auto *descriptorSet = testPool.internallyAllocateDescriptorSet();
////        ASSERT_NE(descriptorSet, nullptr);
//    }
//    ASSERT_TRUE(testPool.isFull());
//    ASSERT_TRUE(testPool.getNumberOfAllocatedDescriptors() == TEST_POOL_SIZE);
//
//    // not sure if there's anything else we should test besides creating the pools
//
//    // test resetting the descriptor pool?
//    ASSERT_NO_THROW(testPool.resetDescriptorPool());
//    ASSERT_TRUE(!testPool.isFull());
//    ASSERT_TRUE(!testPool.getNumberOfAllocatedDescriptors());
//
//    // make sure we destroy the test layout since we defined it manually
//    vkDestroyDescriptorSetLayout(testDevice->getVkDevice(), testLayout, nullptr);
//}
//
//TEST_F(DescriptorSetAllocatorTest,
//       DebugTestWrapperClassesWithRawVulkanCode) {
//    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
//        0,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        1,
//        VK_SHADER_STAGE_ALL_GRAPHICS,
//        nullptr
//    };
//
//    VkBufferCreateInfo testBufferCreateInfo{
//        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//        nullptr,
//        0,
//        10,
//        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//        VK_SHARING_MODE_EXCLUSIVE,
//        1,
//        &testDevice->getGraphicsQueueFamilyIndex()
//    };
//
//    VmaAllocationCreateInfo testBufferAllocationCreateInfo{
//    };
//    testBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//
//    VkBuffer testBuffer = VK_NULL_HANDLE;
//    VmaAllocation testBufferAllocation;
//    ASSERT_EQ(vmaCreateBuffer(testVmaAllocator, &testBufferCreateInfo, &testBufferAllocationCreateInfo, &testBuffer,
//                  &testBufferAllocation, nullptr), VK_SUCCESS);
//
//    VkDescriptorBufferInfo testBufferInfo{
//        testBuffer,
//        0,
//        10
//    };
//
//    set::WriteBinding writeBinding{
//        UniqueIdentifier(),
//        0,
//        0,
//        1,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        boost::none,
//        boost::optional<VkDescriptorBufferInfo>(testBufferInfo),
//        boost::none
//    };
//
//    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
//        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//        nullptr,
//        0,
//        1,
//        &descriptorSetLayoutBinding
//    };
//    VkDescriptorSetLayout testLayout = VK_NULL_HANDLE;
//    auto result = vkCreateDescriptorSetLayout(testDevice->getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr,
//                                              &testLayout);
//    ASSERT_EQ(result, VK_SUCCESS);
//
//
//    pEngine::girEngine::backend::vulkan::descriptor::set::ResourceDescriptorBindings descriptorBindings{
//        {writeBinding},
//        {},
//        {}
//    };
//
//    std::cout << "Current instance handle: " << std::to_string((size_t) testInstance->getVkInstance()) << std::endl;
//
//    VkDescriptorSet debugSet = VK_NULL_HANDLE;
//    VkDescriptorPool debugPool = VK_NULL_HANDLE;
//
//    VkDescriptorPoolSize debugPoolSize = {
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        100
//    };
//
//    VkDescriptorPoolCreateInfo debugPoolCreateInfo{
//        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
//        nullptr,
//        0,
//        10,
//        1,
//        &debugPoolSize
//    };
//    ASSERT_EQ(vkCreateDescriptorPool(testDevice->getVkDevice(), &debugPoolCreateInfo, nullptr, &debugPool),
//              VK_SUCCESS);
//
//    // create descriptor set layout with one simple binding
//    VkDescriptorSetLayoutBinding debugLayoutBinding{
//        0,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        1,
//        VK_SHADER_STAGE_ALL_GRAPHICS,
//        nullptr
//    };
//    VkDescriptorSetLayoutCreateInfo debugLayoutCreateInfo{
//        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//        nullptr,
//        0,
//        1,
//        &debugLayoutBinding
//    };
//    VkDescriptorSetLayout debugLayout = VK_NULL_HANDLE;
//    ASSERT_EQ(
//        vkCreateDescriptorSetLayout(testDevice->getVkDevice(), &debugLayoutCreateInfo, nullptr, &debugLayout),
//        VK_SUCCESS);
//
//
//    set::VulkanDescriptorSet debugWrapperSet = set::VulkanDescriptorSet(set::VulkanDescriptorSet::CreationInput{
//        UniqueIdentifier(),
//        "debugWrapperSet",
//        testDevice->getVkDevice(),
//        debugPool,
//        debugLayout
//    });
//    debugSet = debugWrapperSet.getVkDescriptorSetHandle();
//
//    pool::VulkanDescriptorPool debugWrapperPool = pool::VulkanDescriptorPool(pool::VulkanDescriptorPool::CreationInput{
//        UniqueIdentifier(),
//        "debugWrapperPool",
//        debugLayout,
//        {debugLayoutBinding},
//        testDevice->getVkDevice(),
//        10,
//        10,
//        false,
//        false,
//        true
//    });
//
//    // 03-11 - TEST 1 (for pool): see if we can get the rest of the test to succeed after allocating with the pool
//    // VARIANT 1: use the wrapper-accepting method
//    debugWrapperPool.allocateDescriptorSet(
//        debugWrapperSet); // WORKS after the pool is allocated here (disabling to test)
//
//    // now we test binding the descriptor set...
//    // if this also doesn't work then the problem is not with my wrapper code (which would be infinitely more terrifying)
//    VkDescriptorBufferInfo debugBufferInfo{
//        testBuffer,
//        0,
//        10
//    };
//    VkWriteDescriptorSet debugBufferWrite{
//        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//        nullptr,
//        debugSet,
//        0,
//        0,
//        1,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        nullptr,
//        &debugBufferInfo,
//        nullptr
//    };
//    std::vector<VkWriteDescriptorSet> debugBufferWriteVector = {debugBufferWrite};
//
//    vkUpdateDescriptorSets(testDevice->getVkDevice(),
//                           debugBufferWriteVector.size(),
//                           debugBufferWriteVector.data(),
//                           0,
//                           nullptr
//    );
//
//    // TODO - test binding to a command buffer, maybe some other stuff after that if needed(?)
//
//    vkDestroyDescriptorPool(testDevice->getVkDevice(), debugPool, nullptr);
//
//    vkDestroyDescriptorSetLayout(testDevice->getVkDevice(), debugLayout, nullptr);
//
//    vmaDestroyBuffer(testVmaAllocator, testBuffer, testBufferAllocation);
//}
//
//TEST_F(DescriptorSetAllocatorTest, TestAllocatorFunctionality) {
//    // okay so now we can start trying to do the full "integration" tests with all the descriptor set classes!
//    // this is exciting because once this is ready I can move onto the remainder of the bake code and the
//    // remainder of the frame execution code.
//
//    // there'll be some duplication between these two tests I'm working on, especially while I'm trying to debug
//    // the issues with the descriptor set allocator.
//
//    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
//        0,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        1,
//        VK_SHADER_STAGE_ALL_GRAPHICS,
//        nullptr
//    };
//
//    VkBufferCreateInfo testBufferCreateInfo{
//        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//        nullptr,
//        0,
//        10,
//        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//        VK_SHARING_MODE_EXCLUSIVE,
//        1,
//        &testDevice->getGraphicsQueueFamilyIndex()
//    };
//
//    VmaAllocationCreateInfo testBufferAllocationCreateInfo{
//    };
//    testBufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//
//    VkBuffer testBuffer = VK_NULL_HANDLE;
//    VmaAllocation testBufferAllocation;
//    ASSERT_EQ(vmaCreateBuffer(testVmaAllocator, &testBufferCreateInfo, &testBufferAllocationCreateInfo, &testBuffer,
//                  &testBufferAllocation, nullptr), VK_SUCCESS);
//
//    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
//        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//        nullptr,
//        0,
//        1,
//        &descriptorSetLayoutBinding
//    };
//
//    VulkanDescriptorSetAllocator testAllocator(
//        VulkanDescriptorSetAllocator::CreationInput{
//            0,
//            testDevice->getVkDevice(),
//            descriptorSetLayoutCreateInfo,
//            1
//        }
//    );
//
//    ASSERT_NE(testAllocator.getDescriptorSetLayout(), VK_NULL_HANDLE);
//
//    // okay, now that we have the vulkan-only debug test getting to the point where it successfully updates
//    // and binds the memory...
//    // we can try to create the descriptor set and then give it to a pool for allocation!
//    // I was thinking that it's possible the code for "externally" (ugly terminology) allocating
//    // a descriptor set by handing it to a wrapper pool is flawed.
//
//    // The address issue I thought existed is actually non-existent; the raw Vulkan code seems to work and has that
//    // characteristic when I check values in the debugger.
//
//    // create descriptor pool
//    const std::vector testBindingsVector = {descriptorSetLayoutBinding};
//    pool::VulkanDescriptorPool testPool(pool::VulkanDescriptorPool::CreationInput{
//        UniqueIdentifier(),
//        "debugLabel",
//        testAllocator.getDescriptorSetLayout(),
//        testBindingsVector,
//        testDevice->getVkDevice(),
//        0,
//        10,
//        false, // TODO - get rid of these ugly ass boolean flags
//        false,
//        true
//    });
//
//    // allocate a descriptor set
//
//    VkDescriptorBufferInfo testBufferBindingInfo{
//        testBuffer,
//        0,
//        10
//    };
//
//    set::WriteBinding testBinding{
//        UniqueIdentifier(), // TODO - replace this unique identifier with a template argument that takes a reference
//        0,
//        0,
//        1,
//        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//        boost::none,
//        boost::optional<VkDescriptorBufferInfo>(testBufferBindingInfo),
//        boost::none
//
//    };
//
//    set::ResourceDescriptorBindings testBindings{
//        {testBinding},
//        {},
//        {}
//    };
//
//    // acquire the descriptor set!
//    VkDescriptorSet testDescriptorSet = VK_NULL_HANDLE;
//    ASSERT_NO_THROW(testDescriptorSet = testAllocator.requestDescriptorSet(0, testBindings));
//    ASSERT_NE(testDescriptorSet, VK_NULL_HANDLE);
//
//    // TODO - probably do some more in-depth testing here but as long as it can allocate a DS for now that's fine
//
//    // actually I think I'll do a couple quick tests...
//    testBindings.writeBindings[0].bindingIndex = 1u; // change binding index
//    // !! IMPORTANT !! - always recompute the hash when you change it; i wish i knew how to automate this
//    testBindings.computeHash();
//    auto &newDescriptorSetHandle = testAllocator.requestDescriptorSet(0, testBindings);
//    ASSERT_NE(newDescriptorSetHandle, testDescriptorSet);
//
//    // TODO - test that descriptor pools are correctly detecting fullness
//}
//
//TEST_F(DescriptorSetAllocatorTest, TestReflectedLayoutFromShader) {
//    // this would be another integration test that would verify we can allocate and use
//    // descriptors, maybe in a simple compute shader or something?
//    // we'll see what's reasonable, if it's too much work to actually test the descriptor sets here
//    // then we'll leave it for the frame execution stuff where you're going to be binding actual sets
//    // to actual command buffers that are actually being submitted for execution
//
//    // this will probably be TODO until we get reflection happening elsewhere (doesn't make sense to do it here raw)
//}
