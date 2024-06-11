//
// Created by paull on 2022-08-27.
//

#ifdef _WIN32

#include <gtest/gtest.h>

#include "../../../../../../../src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/Win32Window/Win32Window.hpp"
#include "../../../../../../../src/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterface.hpp"

class Win32WindowTest : public ::testing::Test {
protected:
    void SetUp() override {
//        applicationContext = std::make_shared<PApplicationContext>();
//
//        windowSystem = std::make_shared<VulkanOSInterface>(VulkanOSInterface::CreationInput{
//                OSInterface::WindowPlatform::WINDOWS,
//                applicationContext->getInstance(),
//                applicationContext->getLogicalDevice(),
//                applicationContext->getPhysicalDevice(),
//                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
//                VK_PRESENT_MODE_FIFO_KHR,
////                {TEST_SWAPCHAIN_IMAGE_EXTENT_X, TEST_SWAPCHAIN_IMAGE_EXTENT_Y}
//        });
//
//
//        testWindow = std::make_shared<Win32Window>(Win32Window::CreationInput{
//                "TestWindowClassName",
//                "TestWindowText",
//                windowSystem.get(),
//                true
//        });
    }

//    std::shared_ptr<PApplicationContext> applicationContext = nullptr;
//
//    std::shared_ptr<OSInterface> windowSystem = nullptr;
//
//    std::shared_ptr<Win32Window> testWindow = nullptr;
};

TEST_F(Win32WindowTest, CreateWin32Window) {
//    ASSERT_TRUE(testWindow.get());
//    ASSERT_TRUE(testWindow->getWinApiHWND() != NULL);
}

TEST_F(Win32WindowTest, TestWindowCommands) {
//    ASSERT_TRUE(testWindow->executeWindowCommand(Win32Window::WindowCommand::SHOW) ==
//                Win32Window::ExecuteWindowCommandResult::SUCCESS);
//    ASSERT_TRUE(
//            testWindow->executeWindowCommand(
//                    Win32Window::WindowCommand::MINIMIZE) == Win32Window::ExecuteWindowCommandResult::SUCCESS);
//
//    ASSERT_TRUE(testWindow->executeWindowCommand(Win32Window::WindowCommand::MAXIMIZE) ==
//                Win32Window::ExecuteWindowCommandResult::SUCCESS);
//
//    ASSERT_TRUE(testWindow->executeWindowCommand(Win32Window::WindowCommand::HIDE) ==
//                Win32Window::ExecuteWindowCommandResult::SUCCESS);
}

#endif