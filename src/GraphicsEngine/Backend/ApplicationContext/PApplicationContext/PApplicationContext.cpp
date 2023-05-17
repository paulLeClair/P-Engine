//
// Created by paull on 2022-06-19.
//

#include "PApplicationContext.hpp"

#include <stdexcept>

#include "../WindowSystem/PWindowSystem/PWindowSystem.hpp"

namespace PGraphics {

    const char *PApplicationContext::DEFAULT_NAME = "default"; // TODO - evaluate whether this should be static or a private constant...

    PApplicationContext::PApplicationContext() : vulkanInstance({DEFAULT_NAME}),
                                                 physicalDevice({DEFAULT_NAME, vulkanInstance.getVkInstance()}),
                                                 logicalDevice({DEFAULT_NAME, physicalDevice.getPhysicalDevice()}) {

        imGuiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(imGuiContext);

        PWindowSystem::CreationInput windowSystemCreateInfo = {
                WindowSystem::WindowPlatform::WINDOWS,
                vulkanInstance.getVkInstance(),
                logicalDevice.getLogicalDevice(),
                physicalDevice.getPhysicalDevice(),
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR
        };
        windowSystem = std::make_shared<PWindowSystem>(windowSystemCreateInfo);
    }

    VkRect2D PApplicationContext::getRenderArea() {
        return windowSystem->getRenderArea();
    }

    std::shared_ptr<WindowSystem> PApplicationContext::getWindowSystem() {
        return std::dynamic_pointer_cast<WindowSystem>(windowSystem);
    }

    uint32_t PApplicationContext::getSwapchainImageWidth() {

        return windowSystem->getSwapchainImageWidth();
    }

    uint32_t PApplicationContext::getSwapchainImageHeight() {
        return windowSystem->getSwapchainImageHeight();
    }

}// namespace PGraphics