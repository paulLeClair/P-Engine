//
// Created by paull on 2022-06-28.
//

#include "VulkanOSInterface.hpp"

#include <stdexcept>
#include <utility>

#ifdef _WIN32

#include "../OSWindow/Win32Window/Win32Window.hpp"

#endif

namespace pEngine::girEngine::backend::appContext::osInterface::vulkan {

    VulkanOSInterface::VulkanOSInterface(const CreationInput &creationInput)
            : instance(creationInput.instance),
              logicalDevice(creationInput.logicalDevice),
              physicalDevice(creationInput.physicalDevice),
              surface(VK_NULL_HANDLE),
              vulkanSwapchain(nullptr),
              windowPlatform(creationInput.windowPlatform) {
        build(creationInput);

    }

    void VulkanOSInterface::build(const CreationInput &input) {
#ifdef _WIN32
        createWin32WindowAndSurface(input.initialWidth, input.initialHeight);// TODO - split this into 2 calls
#endif
        createSwapchain(input);

        viewport = {
                0,
                0,
                static_cast<float>(vulkanSwapchain->getSwapchainImageWidth()),
                static_cast<float>(vulkanSwapchain->getSwapchainImageHeight()),
                0.0, // TODO - figure out configuration of these values
                1.0
        };

        scissor = {
                {}, // no offset
                {vulkanSwapchain->getSwapchainImageWidth(), vulkanSwapchain->getSwapchainImageHeight()}
        };

    }


#ifdef _WIN32

    void VulkanOSInterface::createWin32WindowAndSurface(int initialWidth, int initialHeight) {
        osInterface::osWindow::win32::Win32Window::CreationInput win32WindowCreateInfo = {
                "MainEngineWindow",
                "PGame",
                this,
                true,
                initialWidth,
                initialHeight
        };

        window = std::make_shared<osInterface::osWindow::win32::Win32Window>(win32WindowCreateInfo);

        auto hinstance = reinterpret_cast<HINSTANCE>( GetModuleHandle(nullptr));

        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
                VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                nullptr,
                0,
                hinstance,
                std::dynamic_pointer_cast<osInterface::osWindow::win32::Win32Window>(window)->getWinApiHWND()};

        if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create win32 surface!");
        }
    }

#endif

    VulkanOSInterface::~VulkanOSInterface() {
        vkDestroySwapchainKHR(logicalDevice, vulkanSwapchain->getSwapchain(), nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    VkRect2D VulkanOSInterface::getRenderArea() const {
        return {{0,                                         0},
                {vulkanSwapchain->getSwapchainImageWidth(), vulkanSwapchain->getSwapchainImageHeight()}};
    }

    OSInterface::CreateWindowResult VulkanOSInterface::createWindow() {
        return OSInterface::CreateWindowResult::FAILURE;
    }

    OSInterface::ShowWindowResult VulkanOSInterface::showWindow() {
        return (window->executeWindowCommand(osInterface::osWindow::OSWindow::WindowCommand::SHOW) ==
                osInterface::osWindow::OSWindow::ExecuteWindowCommandResult::SUCCESS)
               ? OSInterface::ShowWindowResult::SUCCESS
               : OSInterface::ShowWindowResult::FAILURE;
    }


    OSInterface::ResizeWindowResult VulkanOSInterface::resizeWindow(unsigned int length, unsigned int width) {
//        try {
//            // TODO - evaluate whether this works... resizable windows can probably be a separate issue later on tho
//            VkExtent2D newImageExtent = {length, width};
//            vulkanSwapchain->setSwapchainImageExtent(newImageExtent);
//            build();
//        }
//        catch (std::exception &exception) {
//            return OSInterface::ResizeWindowResult::FAILURE;
//        }

        // TODO - rewrite all this resize window crud

        return OSInterface::ResizeWindowResult::SUCCESS;
    }

    const VkRect2D &VulkanOSInterface::getScissor() const {
        return scissor;
    }

}// namespace PGraphics