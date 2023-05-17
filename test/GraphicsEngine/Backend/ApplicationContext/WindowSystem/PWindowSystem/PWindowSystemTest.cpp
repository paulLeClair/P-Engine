#include <gtest/gtest.h>
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanLogicalDevice/VulkanLogicalDevice.hpp"
#include "../../../../../../src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstance.hpp"

using namespace PGraphics;

class PWindowSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        applicationContext = std::make_shared<PApplicationContext>();
        windowSystem = std::make_shared<PWindowSystem>(PWindowSystem::CreationInput{
                PGraphics::PWindowSystem::WindowPlatform::WINDOWS,
                applicationContext->getInstance(),
                applicationContext->getLogicalDevice(),
                applicationContext->getPhysicalDevice(),
                VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_PRESENT_MODE_FIFO_KHR
        });
    }

    std::shared_ptr<PApplicationContext> applicationContext;
    std::shared_ptr<PWindowSystem> windowSystem;
};

TEST_F(PWindowSystemTest, CreateWindowSystem) {
    ASSERT_TRUE(windowSystem->getSwapchain() != nullptr);
    ASSERT_TRUE(windowSystem->getWindowPlatform() == WindowSystem::WindowPlatform::WINDOWS);
}

// TODO - flesh these tests out
