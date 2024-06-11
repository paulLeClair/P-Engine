#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterface.hpp"
//#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
//#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanLogicalDevice/VulkanLogicalDevice.hpp"
//#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanInstance/VulkanInstance.hpp"


class PWindowSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
//        applicationContext = std::make_shared<PApplicationContext>();
//        windowSystem = std::make_shared<VulkanOSInterface>(VulkanOSInterface::CreationInput{
//                PGraphics::VulkanOSInterface::WindowPlatform::WINDOWS,
//                applicationContext->getInstance(),
//                applicationContext->getLogicalDevice(),
//                applicationContext->getPhysicalDevice(),
//                VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
//                VK_PRESENT_MODE_FIFO_KHR
//        });
    }

//    std::shared_ptr<PApplicationContext> applicationContext;
//    std::shared_ptr<VulkanOSInterface> windowSystem;
};

TEST_F(PWindowSystemTest, CreateWindowSystem) {
//    ASSERT_TRUE(windowSystem->getSwapchain() != nullptr);
//    ASSERT_TRUE(windowSystem->getWindowPlatform() == OSInterface::WindowPlatform::WINDOWS);
}

// TODO - flesh these tests out
