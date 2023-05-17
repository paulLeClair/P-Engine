//
// Created by paull on 2022-06-19.
//

#pragma once

#include <memory>

#include "../../../../lib/dear_imgui/imgui.h"
#include "../../../../lib/dear_imgui/imgui_internal.h"
#include "../ApplicationContext.hpp"
#include "VulkanInstance/VulkanInstance.hpp"
#include "VulkanLogicalDevice/VulkanLogicalDevice.hpp"
#include "VulkanPhysicalDevice/VulkanPhysicalDevice.hpp"

namespace PGraphics {

    class PWindowSystem;

    class PApplicationContext : public ApplicationContext {
    public:
        struct CreationInput {
            // TODO - application context configuration variables would go here
        };

        static const char *DEFAULT_NAME;

        PApplicationContext();

        ~PApplicationContext() = default;

        std::shared_ptr<WindowSystem> getWindowSystem() override;

        VkDevice getLogicalDevice() {
            return logicalDevice.getLogicalDevice();
        }

        VkPhysicalDevice getPhysicalDevice() {
            return physicalDevice.getPhysicalDevice();
        }

        VkInstance getInstance() {
            return vulkanInstance.getVkInstance();
        }

        VkRect2D getRenderArea();

        VkQueue getGraphicsQueue() {
            return logicalDevice.getGraphicsQueue();
        }

        [[nodiscard]] uint32_t getGraphicsQueueFamilyIndex() const {
            return logicalDevice.getGraphicsQueueFamilyIndex();
        }

        [[nodiscard]] uint32_t getComputeQueueFamilyIndex() const {
            return logicalDevice.getComputeQueueFamilyIndex();
        }

    private:
        VulkanInstance vulkanInstance;
        VulkanPhysicalDevice physicalDevice;
        VulkanLogicalDevice logicalDevice;

        std::shared_ptr<PWindowSystem> windowSystem = nullptr;

        ImGuiContext *imGuiContext;

        VkExtent2D getSwapchainImageExtent();

        uint32_t getSwapchainImageWidth();

        uint32_t getSwapchainImageHeight();
    };

}// namespace PGraphics
