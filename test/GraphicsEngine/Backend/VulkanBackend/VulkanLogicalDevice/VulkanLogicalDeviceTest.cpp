//
// Created by paull on 2022-06-22.
//

#include "../../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstance.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"

#include <gtest/gtest.h>

class VulkanLogicalDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
//        testInstance = std::make_shared<VulkanInstance>(VulkanInstance::CreationInput{"test"});
//        testPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(
//                VulkanPhysicalDevice::CreationInput{"test", testInstance->getVkInstance()});
//        testLogicalDevice = std::make_shared<VulkanLogicalDevice>(VulkanLogicalDevice::CreationInput
//                                                                          {"test",
//                                                                           testPhysicalDevice->getPhysicalDevice()});
    }
//
//    std::shared_ptr<VulkanInstance> testInstance;
//
//    std::shared_ptr<VulkanPhysicalDevice> testPhysicalDevice;
//
//    std::shared_ptr<VulkanLogicalDevice> testLogicalDevice;
};

TEST_F(VulkanLogicalDeviceTest, CreateLogicalDevice) {
//    ASSERT_TRUE(testLogicalDevice.get() != nullptr);
//    ASSERT_TRUE(testLogicalDevice->getLogicalDevice() != VK_NULL_HANDLE);
//    ASSERT_TRUE(testLogicalDevice->getGraphicsQueue() != VK_NULL_HANDLE);
}

// TODO - expand this as VulkanLogicalDevice becomes more configurable in the future