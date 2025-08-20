//
// Created by paull on 2025-08-18.
//

#include "VulkanOsInterface.hpp"

#include <stdexcept>

#ifdef _WIN32

#include "../../../Application/OSInterface/OSWindow/Win32Window/Win32Window.hpp"

#endif

namespace pEngine::girEngine::backend::vulkan {
    vulkan::VulkanOSInterface::VulkanOSInterface(const CreationInput &creationInput)
        : instance(creationInput.instance),
          logicalDevice(creationInput.logicalDevice),
          physicalDevice(creationInput.physicalDevice),
          surface(VK_NULL_HANDLE),
          windowPlatform(creationInput.windowPlatform) {
        build(creationInput);
    }

    void vulkan::VulkanOSInterface::build(const CreationInput &input) {
#ifdef _WIN32
        createWin32WindowAndSurface(input.initialWidth, input.initialHeight); // TODO - split this into 2 calls
#endif

#ifdef __linux__
        createXLibWindowAndSurface(input.initialWidth, input.initialHeight);
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

    void VulkanOSInterface::createWin32WindowAndSurface(
        int initialWidth, int initialHeight) {
        app::Win32Window::CreationInput win32WindowCreateInfo = {
            "MainEngineWindow",
            "PGame",
            this,
            true,
            initialWidth,
            initialHeight
        };

        window = std::make_shared<app::Win32Window>(win32WindowCreateInfo);

        auto hinstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(nullptr));

        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
            VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            nullptr,
            0,
            hinstance,
            std::dynamic_pointer_cast<app::Win32Window>(window)->getWinApiHWND()
        };

        if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create win32 surface!");
        }
    }

#endif

#ifdef __linux__

    void VulkanOSInterface::createXLibWindowAndSurface(int initialWidth, int initialHeight) {
        // create x11 display (server?) not sure how x11 works in detail lol
        x11Display = XOpenDisplay(nullptr);
        if (!x11Display) {
            // TODO - log!
        }

        window = std::make_shared<osWindow::xlib::XLibWindow>(osWindow::xlib::XLibWindow::CreationInput{
                x11Display,
                static_cast<unsigned int>(initialWidth),
                static_cast<unsigned int>(initialHeight),
                0,
                0,
                0 // no clue what this background value should be
        });

        VkXlibSurfaceCreateInfoKHR xlibSurface{
                VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                nullptr,
                0,
                x11Display,
                std::dynamic_pointer_cast<osWindow::xlib::XLibWindow>(window)->getX11Window()
        };

        auto result = vkCreateXlibSurfaceKHR(instance, &xlibSurface, nullptr, &surface);
        if (result != VK_SUCCESS) {
            // TODO - log!
        }
    }

#endif

    vulkan::VulkanOSInterface::~VulkanOSInterface() {
        vkDestroySwapchainKHR(logicalDevice, vulkanSwapchain->getSwapchain(), nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    VkRect2D vulkan::VulkanOSInterface::getRenderArea() const {
        return {
            {0, 0},
            {vulkanSwapchain->getSwapchainImageWidth(), vulkanSwapchain->getSwapchainImageHeight()}
        };
    }

    app::OSInterface::CreateWindowResult vulkan::VulkanOSInterface::createWindow() {
        return app::OSInterface::CreateWindowResult::FAILURE;
    }

    app::OSInterface::ShowWindowResult vulkan::VulkanOSInterface::showWindow() {
        return (window->executeWindowCommand(app::OSWindow::WindowCommand::SHOW) ==
                app::OSWindow::ExecuteWindowCommandResult::SUCCESS)
                   ? app::OSInterface::ShowWindowResult::SUCCESS
                   : app::OSInterface::ShowWindowResult::FAILURE;
    }


    app::OSInterface::ResizeWindowResult vulkan::VulkanOSInterface::resizeWindow(
        unsigned int length, unsigned int width) {
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

    const VkRect2D &vulkan::VulkanOSInterface::getScissor() const {
        return scissor;
    }
} // namespace PGraphics
