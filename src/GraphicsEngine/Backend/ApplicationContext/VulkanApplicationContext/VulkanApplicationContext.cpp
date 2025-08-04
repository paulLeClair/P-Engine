//
// Created by paull on 2022-06-19.
//

#include "VulkanApplicationContext.hpp"

namespace pEngine::girEngine::backend::appContext::vulkan {
    const char *VulkanApplicationContext::DEFAULT_NAME = "default";
    // TODO - evaluate whether this should be static or a private constant...

    VulkanApplicationContext::VulkanApplicationContext(const CreationInput &creationInput)
        : vulkanInstance(
              std::make_shared<VulkanInstance>(VulkanInstance::CreationInput{
                  creationInput.enabledInstanceExtensions,
                  creationInput.enabledInstanceLayers,
                  creationInput.vulkanAppName,
                  creationInput.vulkanEngineName,
                  creationInput.appVersion,
                  creationInput.engineVersion,
                  creationInput.vulkanApiVersion
              })),
          physicalDevice(
              std::make_shared<VulkanPhysicalDevice>(VulkanPhysicalDevice::CreationInput{
                  util::UniqueIdentifier(),
                  creationInput.physicalDeviceLabel,
                  vulkanInstance->getVkInstance(),
                  PhysicalDeviceChoice::BEST_GPU,
                  VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
                  creationInput.vulkanApiVersion
              })),
          logicalDevice(
              std::make_shared<VulkanLogicalDevice>(VulkanLogicalDevice::CreationInput{
                  creationInput.logicalDeviceLabel,
                  physicalDevice->getPhysicalDevice(),
                  creationInput.enabledDeviceExtensions,
                  creationInput.enabledDeviceLayers
              })) {
        // TODO - redo the window system stuff (remove all the hardcoded stuff and what not)
        osInterface::vulkan::VulkanOSInterface::CreationInput osInterfaceCreateInfo = {
            osInterface::OSInterface::WindowPlatform::WINDOWS,
            vulkanInstance->getVkInstance(),
            logicalDevice->getVkDevice(),
            physicalDevice->getPhysicalDevice(),
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // TODO - support alpha stuff
            creationInput.swapchainPresentMode,
            creationInput.numberOfSwapchainImages,
            obtainSwapchainImageFormat(creationInput.desiredSwapchainImageFormat),
            creationInput.swapchainImageColorSpace,
            creationInput.swapchainImageUsages,
            creationInput.swapchainImagePreTransform,
            creationInput.swapchainShouldClipObscuredTriangles,
            creationInput.initialWindowWidth,
            creationInput.initialWindowHeight
        };
        osInterface = std::make_shared<osInterface::vulkan::VulkanOSInterface>(osInterfaceCreateInfo);
        imGuiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(imGuiContext);
    }

    VkFormat VulkanApplicationContext::obtainSwapchainImageFormat(VkFormat desiredFormat) {
        // OKAY - so here's probably where we can test whether a desired format works or not;
        // maybe to simplify things for now I'll just skip the check cause it's basically just boilerplate

        return desiredFormat;
    }
}
