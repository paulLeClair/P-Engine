#include <iostream>

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1

#include <windows.h>

#endif

// application includes
#include "Application/Application.hpp"
#include "EngineCore/EngineCore.hpp"
#include "GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"
#include "Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineMode.hpp"
#include "Application/EngineMode/AnimatedModelDemoMode/AnimatedModelDemoMode.hpp"

using namespace pEngine;
using namespace pEngine::app;
using namespace pEngine::core;

void runApplication(const std::string &modelFilePath) {
    const auto scene = std::make_shared<Scene>(Scene::CreationInput{
        "Animated Model Demo scene",
        "Animated Model Demo Render Graph"
    });

    const auto backendConfig = backend::vulkan::VulkanBackend::CreationInput{
        "Animated Model Viewer Demo",
        "GirEngine",
        nullptr, // ignore threadpool for now
        {
            backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::SURFACE_EXTENSION,
#ifdef _WIN32
            backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::WINDOWS_SURFACE_EXTENSION
#endif
#ifdef __linux__
                    backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::XLIB_SURFACE_EXTENSION
#endif
        },
        {
#ifndef DISABLE_VALIDATION_LAYER
            // re-enable validation layers while i just mess around with it (pre nsight capture)
            backend::appContext::vulkan::VulkanInstance::SupportedLayers::VALIDATION_LAYER
#endif
        },
        {
            backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::SWAPCHAIN_EXTENSION,
            backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::SYNC_2,
            backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::DYNAMIC_RENDERING,
            backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::NONSEMANTIC_SHADER_INFO,
        },
        {},
        "Animated Model Demo - physical device",
        "Animated Model - logical device",
        VK_MAKE_VERSION(0, 1, 1),
        VK_MAKE_VERSION(0, 1, 1),
        VK_MAKE_API_VERSION(0, 1, 3,
                            275),
        // TODO - find a way to not have to specify the exact version number (unless its unavoidable),
        // SWAPCHAIN CONFIGURATION
        VK_PRESENT_MODE_MAILBOX_KHR,
        2,
        VK_FORMAT_B8G8R8A8_SRGB, // arbitrary SRGB format
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, // SRGB color space to go with srgb format
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, // no pre-transform
        true, // enable clipping,
        1900,
        1080
    };
    const auto backend = std::make_shared<backend::vulkan::VulkanBackend>(backendConfig);

    const auto engineCore = std::make_shared<EngineCore<Scene, backend::vulkan::VulkanBackend> >(
        EngineCore<Scene, backend::vulkan::VulkanBackend>::CreationInput{
            "Engine Core for Animated Model Demo",
            1, // single-threading for now
            scene,
            backend
        }
    );

    // at this point we should be able to create the engine mode itself
    const auto demoMode = std::make_shared<mode::AnimatedModelDemoMode<Scene, backend::vulkan::VulkanBackend> >(
        mode::AnimatedModelDemoMode<Scene, backend::vulkan::VulkanBackend>::CreationInput{
            "Animated Models Demo - Engine Mode",
            engineCore,
            nullptr,
            std::filesystem::path(
                modelFilePath
            )
        }
    );

    demoMode->begin();
}

#ifdef _WIN32

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    std::cout << "GirEngine, Activate!" << std::endl;

    const std::string modelFilePath = lpCmdLine;
    runApplication(modelFilePath);

    return 0;
}

#endif

#ifdef __linux__

// LINUX/MAC(?) ENTRY POINT
int main(int argc, char *argv[]) {

    std::cout << "Pee Engine, Activate!" << std::endl;

    runEngine();

    return 0;
}

#endif
