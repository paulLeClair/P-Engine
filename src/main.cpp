#include <iostream>

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1

#include <windows.h>

// this macro will be used to control whether the main actually runs the application or just returns
#define RUN_APPLICATION_WINDOW

#ifdef RUN_APPLICATION_WINDOW
// application includes
#include "Application/Application.hpp"
#include "EngineCore/EngineCore.hpp"
#include "GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp"
#include "Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineMode.hpp"


using namespace pEngine;
using namespace pEngine::app;
using namespace pEngine::core;

#endif


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {

    std::cout << "Pee Engine, Activate!" << std::endl;

#ifdef RUN_APPLICATION_WINDOW

    auto scene = std::make_shared<scene::Scene>(scene::Scene::CreationInput{
            "Core Menu Demo scene",
            "Core Menu Demo Render Graph"
    });

    auto backend = std::make_shared<backend::vulkan::VulkanBackend>(backend::vulkan::VulkanBackend::CreationInput{
            "Core Menu Demo",
            "GirEngine",
            nullptr, // ignore threadpool for now
            {
                    backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::SURFACE_EXTENSION,
                    backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::WINDOWS_SURFACE_EXTENSION},
            {
                    backend::appContext::vulkan::VulkanInstance::SupportedLayers::VALIDATION_LAYER
            },
            {
                    backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::SWAPCHAIN_EXTENSION,
                    backend::appContext::vulkan::VulkanLogicalDevice::SupportedDeviceExtension::SYNC_2
            },
            {},
            "Core Menu Demo - physical device",
            "Core Menu Demo - logical device",
            VK_MAKE_VERSION(0, 1, 1),
            VK_MAKE_VERSION(0, 1, 1),
            VK_MAKE_API_VERSION(0, 1, 3,
                                275), // TODO - find a way to not have to specify the exact version number (unless its unavoidable),
            // SWAPCHAIN CONFIGURATION
            VK_PRESENT_MODE_FIFO_KHR,
            3, // triple buffering
            VK_FORMAT_B8G8R8A8_SRGB, // arbitrary SRGB format - not sure if it'll work
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, // SRGB color space to go with srgb format
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT, // arbitrary usages (not sure if we need anything but color?)
            VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, // no pre-transform
            true, // enable clipping,
            800, // placeholder initial window size lol
            600
    });

    auto engineCore = std::make_shared<EngineCore<scene::Scene, backend::vulkan::VulkanBackend>>(
            EngineCore<scene::Scene, backend::vulkan::VulkanBackend>::CreationInput{
                    "Engine Core for Core Menu Demo",
                    util::UniqueIdentifier(),
                    1, // single-threading for now
                    scene,
                    backend
            }
    );

    // at this point we should be able to create the engine mode itself
    auto coreMenuMode = std::make_shared<mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend>>(
            mode::CoreMenuEngineMode<scene::Scene, backend::vulkan::VulkanBackend>::CreationInput{
                    "Core Menu Demo - Engine Mode",
                    util::UniqueIdentifier(),
                    engineCore,
                    nullptr,
                    {}, // no callbacks for now
                    false // run the main loop
            }
    );

    // temporary try-catch to begin the mode
    try {
        coreMenuMode->begin();
    }
    catch (std::exception &e) {
        std::cout << "Pee Engine has died! Reason: " << e.what() << std::endl;
    }

#endif

    return 0;
}

#endif

#ifdef __linux__
// LINUX/MAC(?) ENTRY POINT
int main( int argc, char *argv[] ) {

  std::cout << "Pee Engine, Activate!" << std::endl;

  return 0;
}
#endif