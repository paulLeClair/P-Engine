//
// Created by paull on 2024-03-16.
//

#include <gtest/gtest.h>

#include "../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"
#include "../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocator.hpp"

using namespace pEngine::girEngine::backend;
using namespace pEngine::girEngine::gir;

class VulkanBufferSuballocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        constexpr unsigned VULK_API_VERSION = VK_MAKE_API_VERSION(0, 1, 3, 0);

        testInstance = std::make_shared<appContext::vulkan::VulkanInstance>(
                appContext::vulkan::VulkanInstance::CreationInput{
                        {},
                        {appContext::vulkan::VulkanInstance::SupportedLayers::VALIDATION_LAYER},
                        "test",
                        "test",
                        VK_MAKE_VERSION(0, 0, 1),
                        VK_MAKE_VERSION(0, 0, 1),
                        VULK_API_VERSION
                });

        testPhysicalDevice = std::make_shared<appContext::vulkan::VulkanPhysicalDevice>(
                appContext::vulkan::VulkanPhysicalDevice::CreationInput{
                        UniqueIdentifier(),
                        "testPhysicalDevice",
                        testInstance->getVkInstance(),
                        appContext::vulkan::PhysicalDeviceChoice::BEST_GPU,
                        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                        VULK_API_VERSION
                });

        testDevice = std::make_shared<appContext::vulkan::VulkanLogicalDevice>(
                appContext::vulkan::VulkanLogicalDevice::CreationInput{
                        "testDevice",
                        testPhysicalDevice->getPhysicalDevice()
                });

        const VmaAllocatorCreateInfo allocatorCreateInfo{
                0,
                testPhysicalDevice->getPhysicalDevice(),
                testDevice->getVkDevice(),
                0,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                testInstance->getVkInstance(),
                VULK_API_VERSION
        };
        ASSERT_EQ(vmaCreateAllocator(&allocatorCreateInfo, &allocator), VK_SUCCESS);
    }

    std::shared_ptr<appContext::vulkan::VulkanInstance> testInstance;
    std::shared_ptr<appContext::vulkan::VulkanPhysicalDevice> testPhysicalDevice;
    std::shared_ptr<appContext::vulkan::VulkanLogicalDevice> testDevice;

    VmaAllocator allocator = VK_NULL_HANDLE;
};

TEST_F(VulkanBufferSuballocatorTest, BasicTests) {
    // Here we wanna just do a couple quick and dirty tests...

    constexpr VkBufferUsageFlags TEST_FLAGS = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    constexpr unsigned TEST_BUFFER_SIZE = 64;

    std::vector<unsigned char> bufferData(TEST_BUFFER_SIZE);
    std::vector<unsigned char> anotherBufferData(TEST_BUFFER_SIZE);

    for (unsigned i = 0; i < TEST_BUFFER_SIZE; i++) {
        bufferData[i] = 'a';
        anotherBufferData[i] = 'b';
    }

    auto testBufferGir = std::make_shared<BufferIR>(
            BufferIR::CreationInput{
                    "testBuffer",
                    UniqueIdentifier(),
                    GIRSubtype::BUFFER,
                    BufferIR::BufferUsage::UNIFORM_BUFFER,
                    bufferData.data(),
                    bufferData.size() * sizeof(unsigned char)
            }
    );

    auto anotherTestBufferGir = std::make_shared<BufferIR>(
            BufferIR::CreationInput{
                    "testBuffer2",
                    UniqueIdentifier(),
                    GIRSubtype::BUFFER,
                    BufferIR::BufferUsage::STORAGE_BUFFER,
                    anotherBufferData.data(),
                    anotherBufferData.size() * sizeof(unsigned char)
            }
    );

    auto testSuballocator = pEngine::girEngine::backend::vulkan::VulkanBufferSuballocator(
            pEngine::girEngine::backend::vulkan::VulkanBufferSuballocator::CreationInput{
                    allocator,
                    TEST_FLAGS,
                    {testBufferGir},
                    {testDevice->getGraphicsQueueFamilyIndex()},
                    0.3,
            }
    );

    // test suballocation of original buffer worked
    auto testBufferSuballocation = testSuballocator.findSuballocation(testBufferGir->getUid());
    ASSERT_NE(testBufferSuballocation, nullptr);
    ASSERT_EQ(*testBufferSuballocation->getData(),
              *testBufferGir->getRawDataContainer().getRawDataAsVectorOfBytes().data());

    // test suballocation of an additional buffer
    ASSERT_NO_THROW(testSuballocator.suballocateBuffer(anotherTestBufferGir));
    auto anotherTestBufferSuballocation = testSuballocator.findSuballocation(anotherTestBufferGir->getUid());
    ASSERT_NE(*testBufferSuballocation->getData(), *anotherTestBufferSuballocation->getData());

    // TODO - test updating buffers and what not
}

TEST_F(VulkanBufferSuballocatorTest, TestDataTransfer) {
    //TODO
}
