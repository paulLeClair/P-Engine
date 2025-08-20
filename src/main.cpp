
#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1

#include <windows.h>

#endif

#include <iostream>
#include "Application/Application.hpp"
#include "GirEngine/Backend/VulkanBackend/VulkanBackend.hpp"
#include "Application/EngineMode/AnimatedModelDemoMode/AnimatedModelDemoMode.hpp"

using namespace pEngine::girEngine::backend::vulkan;

void runApplication(const std::string &modelFilePath) {
    const auto scene = std::make_shared<Scene>(Scene::CreationInput{
        "Animated Model Demo scene",
        "Animated Model Demo Render Graph"
    });

    const auto backendConfig = VulkanBackend::CreationInput{
        "Animated Model Viewer Demo",
        "GirEngine",
        nullptr, // ignore threadpool for now
        {
            VulkanInstance::SupportedInstanceExtension::SURFACE_EXTENSION,
#ifdef _WIN32
            VulkanInstance::SupportedInstanceExtension::WINDOWS_SURFACE_EXTENSION
#endif
#ifdef __linux__
                    backend::appContext::vulkan::VulkanInstance::SupportedInstanceExtension::XLIB_SURFACE_EXTENSION
#endif
        },
        {
#ifndef DISABLE_VALIDATION_LAYER
            // re-enable validation layers while i just mess around with it (pre nsight capture)
            VulkanInstance::SupportedLayers::VALIDATION_LAYER
#endif
        },
        {
            VulkanLogicalDevice::SupportedDeviceExtension::SWAPCHAIN_EXTENSION,
            VulkanLogicalDevice::SupportedDeviceExtension::SYNC_2,
            VulkanLogicalDevice::SupportedDeviceExtension::DYNAMIC_RENDERING,
            VulkanLogicalDevice::SupportedDeviceExtension::NONSEMANTIC_SHADER_INFO,
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
    const auto backend = std::make_shared<VulkanBackend>(backendConfig);

    // at this point we should be able to create the engine mode itself
    auto demoMode = pEngine::app::mode::AnimatedModelDemoMode(
        pEngine::app::mode::AnimatedModelDemoMode::CreationInput{
            "Animated Models Demo - Engine Mode",
            UniqueIdentifier(),
            std::filesystem::path(
                modelFilePath
            ),
            *scene,
            *backend
        }
    );

    demoMode.begin();
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
