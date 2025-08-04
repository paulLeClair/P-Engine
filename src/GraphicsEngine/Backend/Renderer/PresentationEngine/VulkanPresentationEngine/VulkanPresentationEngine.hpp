//
// Created by paull on 2024-05-23.
//

#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "../PresentationEngine.hpp"
#include "../../../ApplicationContext/OSInterface/VulkanOSInterface/VulkanSwapchain/VulkanSwapchain.hpp"

namespace pEngine::girEngine::backend::vulkan {
    /**
     * I guess actually I'll try just making it super-shallow; in the future I'll implement other present modes
     * in a similar way and factor out any commonalities into a base class.
     *
     * For now, we'll just aim to support simple FIFO presentation here. I won't even bother adding it to the name yet
     */
    class VulkanPresentationEngine {
    public:
        struct CreationInput {
            VkDevice device;

            VkQueue presentQueue;

            std::shared_ptr<appContext::osInterface::swapchain::vulkan::VulkanSwapchain> swapchain;

            uint64_t timeoutInMs;

            VkSemaphore imageAcquiredSemaphore;
        };

        explicit VulkanPresentationEngine(const CreationInput &creationInput)
            : device(creationInput.device),
              swapchain(creationInput.swapchain),
              acquireImageTimeoutInMs(creationInput.timeoutInMs),
              imageAcquiredSemaphore(creationInput.imageAcquiredSemaphore),
              presentQueue(creationInput.presentQueue) {
            defaultRenderArea.extent = swapchain->getSwapchainImageExtent();
            defaultRenderArea.offset = VkOffset2D(0, 0); // not sure if this offset has to be anything
        }

        ~VulkanPresentationEngine() = default;

        boost::optional<unsigned> acquireNextSwapchainImageIndex(const VkSemaphore signalSemaphore) {
            unsigned nextImageIndex = 0u;
            auto result = vkAcquireNextImageKHR(
                device,
                swapchain->getSwapchain(),
                acquireImageTimeoutInMs,
                signalSemaphore,
                VK_NULL_HANDLE,
                &nextImageIndex
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
                return boost::none;
            }
            return {nextImageIndex};
        }

        bool presentSwapchainImage(unsigned int swapchainImageIndex, std::vector<VkSemaphore> waitSemaphores) const {
            if (waitSemaphores.empty()) {
                // TODO -> log a warning
            }

            VkPresentInfoKHR presentInfo{
                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                nullptr,
                static_cast<uint32_t>(waitSemaphores.size()),
                waitSemaphores.data(),
                1,
                &swapchain->getSwapchain(),
                &swapchainImageIndex,
                nullptr
            };
            return vkQueuePresentKHR(presentQueue, &presentInfo) == VK_SUCCESS;
        }

        [[nodiscard]] const VkExtent2D &getSwapchainImageExtent() const {
            return swapchain->getSwapchainImageExtent();
        }

        const VkRect2D &getDefaultSwapchainImageRenderArea() {
            return defaultRenderArea;
        }

        [[nodiscard]] appContext::osInterface::swapchain::vulkan::VulkanSwapchain &getSwapchain() {
            return *swapchain;
        }

    private:
        VkDevice device;

        std::shared_ptr<appContext::osInterface::swapchain::vulkan::VulkanSwapchain> swapchain;

        uint64_t acquireImageTimeoutInMs;

        /**
         * For the single-animated-model demo we don't need much more than just a simple default;
         * in the future we can add a bunch of configuration stuff but it'll be easier to verify it works
         * when we have our first "traditional graphics pipeline" implementation running (which is what this demo is)
         */
        VkRect2D defaultRenderArea = {};

        VkSemaphore imageAcquiredSemaphore;

        VkQueue presentQueue;
    };
}
