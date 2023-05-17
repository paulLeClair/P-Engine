//
// Created by paull on 2022-06-28.
//

#include "PWindowSystem.hpp"

#include <stdexcept>
#include <utility>

#ifdef _WIN32

#include "../OSWindow/Win32Window/Win32Window.hpp"

#endif

namespace PGraphics {

    PWindowSystem::PWindowSystem(const CreationInput &creationInput)
            : instance(creationInput.instance),
              logicalDevice(creationInput.logicalDevice),
              physicalDevice(creationInput.physicalDevice),
              swapchainPresentMode(creationInput.swapchainPresentMode),
              swapchainCompositeAlphaFlag(creationInput.compositeAlphaFlag),
              surface(VK_NULL_HANDLE),
              swapchain(VK_NULL_HANDLE),
//              swapchainImageExtent(creationInput.swapchainImageExtent),
              windowPlatform(creationInput.windowPlatform) {
        build();

    }

    void PWindowSystem::initializeSwapchainImageExtent() {
        const VkExtent2D &minimumSwapchainImageExtent = getMinimumSwapchainImageExtent();
        swapchainImageExtent = {minimumSwapchainImageExtent.width, minimumSwapchainImageExtent.height};
    }

    void PWindowSystem::build() {
#ifdef _WIN32
        createWin32WindowAndSurface();// TODO - split this into 2 calls
#endif

        initializeSwapchainImageExtent();

        createSwapchain();

        acquireSwapchainImages();
    }

    void PWindowSystem::createWin32WindowAndSurface() {
        Win32Window::CreationInput win32WindowCreateInfo = {"MainEngineWindow", "PGame", this, true};

        osWindow = std::make_shared<Win32Window>(win32WindowCreateInfo);

        auto hinstance = reinterpret_cast<HINSTANCE>( GetModuleHandle(nullptr));

        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {
                VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                nullptr,
                0,
                hinstance,
                std::dynamic_pointer_cast<Win32Window>(osWindow)->getWinApiHWND()};

        if (vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create win32 surface!");
        }
    }

    PWindowSystem::CreateSwapchainResult
    PWindowSystem::createSwapchain() {
        if (osWindow == nullptr) {
            return PWindowSystem::CreateSwapchainResult::FAILURE;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                nullptr,
                0,
                surface,
                3,// TODO - change this hardcoded triple-buffering value
                getSwapchainImageFormat(),
                getSwapchainImageColorSpace(),
                swapchainImageExtent,
                1,
                getSwapchainImageUsages(),
                VK_SHARING_MODE_EXCLUSIVE,// TODO - change this if we ever support concurrent stuff (i'm ignoring it for now)
                0,
                nullptr,
                getSwapchainPreTransform(),
                swapchainCompositeAlphaFlag,
                swapchainPresentMode,
                VK_TRUE,
                VK_NULL_HANDLE};

        VkResult createSwapchainResult = vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, nullptr, &swapchain);
        if (createSwapchainResult != VK_SUCCESS) {
            return PWindowSystem::CreateSwapchainResult::FAILURE;
        }
        return PWindowSystem::CreateSwapchainResult::SUCCESS;
    }

    PWindowSystem::AcquireSwapchainImagesResult PWindowSystem::acquireSwapchainImages() {
        uint32_t swapchainImageCount = 0;
        auto result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, nullptr);

        if (result != VK_SUCCESS) {
            return PWindowSystem::AcquireSwapchainImagesResult::FAILURE;
        }

        swapchainImages.resize(swapchainImageCount);
        result = vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, swapchainImages.data());

        if (result != VK_SUCCESS) {
            return PWindowSystem::AcquireSwapchainImagesResult::FAILURE;
        }

        return PWindowSystem::AcquireSwapchainImagesResult::SUCCESS;
    }

    PWindowSystem::~PWindowSystem() {
        vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    VkRect2D PWindowSystem::getRenderArea() const {
        return {{0, 0}, swapchainImageExtent};
    }

    WindowSystem::CreateWindowResult PWindowSystem::createWindow() {
        return WindowSystem::CreateWindowResult::FAILURE;
    }

    WindowSystem::ShowWindowResult PWindowSystem::showWindow() {
        return (osWindow->executeWindowCommand(OSWindow::WindowCommand::SHOW) ==
                OSWindow::ExecuteWindowCommandResult::SUCCESS)
               ? WindowSystem::ShowWindowResult::SUCCESS
               : WindowSystem::ShowWindowResult::FAILURE;
    }

    VkImageUsageFlags PWindowSystem::getSwapchainImageUsages() {
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_STORAGE_BIT;// allow rendering into swapchain images or writing from shaders (maybe disable the
        // latter later...?)
    }

    VkSurfaceTransformFlagBitsKHR PWindowSystem::getSwapchainPreTransform() {
        return VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }

    VkExtent2D
    PWindowSystem::getMaximumSwapchainImageExtent() {
        // get physical device surface capabilities, use that to determine the extent
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to get physical device surface capabilities!");
        }

        return surfaceCapabilities.maxImageExtent;
    }

    VkExtent2D PWindowSystem::getMinimumSwapchainImageExtent() {
// get physical device surface capabilities, use that to determine the extent
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to get physical device surface capabilities!");
        }

        return surfaceCapabilities.minImageExtent;
    }


    VkFormat
    PWindowSystem::getSwapchainImageFormat() {

        std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats = getPhysicalDeviceSurfaceFormats();

        // default behavior here: choose the first VkSurfaceFormatKHR that's available
        return physicalDeviceSurfaceFormats.front().format;
    }

    VkColorSpaceKHR
    PWindowSystem::getSwapchainImageColorSpace() {
        std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats = getPhysicalDeviceSurfaceFormats();

        // default behavior here: choose the first VkSurfaceFormatKHR that's available
        return physicalDeviceSurfaceFormats.front().colorSpace;
    }

    std::vector<VkSurfaceFormatKHR>
    PWindowSystem::getPhysicalDeviceSurfaceFormats() const {
        uint32_t numberOfSurfaceFormats = 0;
        auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numberOfSurfaceFormats,
                                                           nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to get physical device surface formats!");
        }

        std::vector<VkSurfaceFormatKHR> physicalDeviceSurfaceFormats(numberOfSurfaceFormats);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                                      surface,
                                                      &numberOfSurfaceFormats,
                                                      physicalDeviceSurfaceFormats.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to get physical device surface formats!");
        }
        return physicalDeviceSurfaceFormats;
    }

    VkImage PWindowSystem::getSwapchainImage(unsigned int frameIndex) {
        return swapchainImages[frameIndex];
    }


    WindowSystem::ResizeWindowResult PWindowSystem::resizeWindow(unsigned int length, unsigned int width) {
        try {
            VkExtent2D newImageExtent = {length, width};
            setSwapchainImageExtent(newImageExtent);
            build();
        }
        catch (std::exception &exception) {
            return WindowSystem::ResizeWindowResult::FAILURE;
        }

        return WindowSystem::ResizeWindowResult::SUCCESS;
    }

}// namespace PGraphics