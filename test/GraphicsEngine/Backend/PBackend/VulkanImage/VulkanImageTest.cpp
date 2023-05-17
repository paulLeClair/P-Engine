//
// Created by paull on 2022-07-01.
//

#include "../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstance.hpp"
#include "../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanLogicalDevice/VulkanLogicalDevice.hpp"
#include "../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
#include "../../../../../src/GraphicsEngine/Backend/PBackend/VulkanImage/VulkanImage.hpp"
#include <gtest/gtest.h>

// TODO - expand this to cover the various ways we'd want to create images;
// TODO 2 - make sure all the different input cases are tested (all the Vulkan-facing objects should be extremely well-tested so the backend can be a reliable black box)

// for now, we'll just test that a basic image can be created

TEST(VulkanImageTestSuite, CreateSimpleImage) {
    PGraphics::VulkanInstance testInstanceManager = PGraphics::VulkanInstance({"test"});
    PGraphics::VulkanPhysicalDevice testPhysicalDeviceManager = PGraphics::VulkanPhysicalDevice(
            {"test", testInstanceManager.getVkInstance()});
    PGraphics::VulkanLogicalDevice testLogicalDeviceManager = PGraphics::VulkanLogicalDevice(
            {"test", testPhysicalDeviceManager.getPhysicalDevice()});

    VmaAllocatorCreateInfo allocatorCreateInfo = {
            0,
            testPhysicalDeviceManager.getPhysicalDevice(),// hardcoded for single default physical device
            testLogicalDeviceManager.getLogicalDevice(),
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            testInstanceManager.getVkInstance(),
            VK_MAKE_VERSION(1, 3, 0)};

    VmaAllocator testAllocator;
    if (vmaCreateAllocator(&allocatorCreateInfo, &testAllocator) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create VmaAllocator!");
    }

//    PGraphics::VulkanImage::CreationInput createInfo;
}
