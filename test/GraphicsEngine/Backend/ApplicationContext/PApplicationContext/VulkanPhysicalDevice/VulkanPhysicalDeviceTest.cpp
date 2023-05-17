//
// Created by paull on 2022-06-22.
//

#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstance.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
#include <gtest/gtest.h>

using namespace PGraphics;

class VulkanPhysicalDeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
        testInstance = std::make_shared<VulkanInstance>(VulkanInstance::CreationInput{
                "test"
        });

        testPhysicalDevice = std::make_shared<VulkanPhysicalDevice>(VulkanPhysicalDevice::CreationInput{
                "test",
                testInstance->getVkInstance()
        });
    }

    std::shared_ptr<VulkanInstance> testInstance;

    std::shared_ptr<VulkanPhysicalDevice> testPhysicalDevice;
};


TEST_F(VulkanPhysicalDeviceTest, CreatePhysicalDevice) {
    ASSERT_TRUE(testPhysicalDevice.get() != nullptr);
    ASSERT_TRUE(testPhysicalDevice->getPhysicalDevice() != VK_NULL_HANDLE);
}

// TODO - extend this as VulkanPhysicalDevice becomes more configurable in the future