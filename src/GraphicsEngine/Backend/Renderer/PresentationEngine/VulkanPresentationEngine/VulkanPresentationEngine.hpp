//
// Created by paull on 2024-05-23.
//

#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "../PresentationEngine.hpp"
#include "../../../ApplicationContext/OSInterface/VulkanOSInterface/VulkanSwapchain/VulkanSwapchain.hpp"

namespace pEngine::girEngine::backend::render::present::vulkan {
    /**
     * I guess actually I'll try just making it super-shallow; in the future I'll implement other present modes
     * in a similar way and factor out any commonalities into a base class.
     *
     * For now, we'll just aim to support simple FIFO presentation here. I won't even bother adding it to the name yet
     */
    class VulkanPresentationEngine : public present::PresentationEngine {
    public:
        struct CreationInput {
            VkDevice device;

            VkQueue presentQueue;

            std::shared_ptr<appContext::osInterface::swapchain::vulkan::VulkanSwapchain> swapchain;

            uint64_t timeoutInMs;

            VkSemaphore imageAcquiredSemaphore;
            VkSemaphore renderCompleteSemaphore;

        };

        explicit VulkanPresentationEngine(const CreationInput &creationInput)
                : device(creationInput.device),
                  swapchain(creationInput.swapchain),
                  presentQueue(creationInput.presentQueue),
                  acquireImageTimeoutInMs(creationInput.timeoutInMs),
                  imageAcquiredSemaphore(creationInput.imageAcquiredSemaphore),
                  renderCompleteSemaphore(creationInput.renderCompleteSemaphore) {

        }

        ~VulkanPresentationEngine() override = default;

        boost::optional<unsigned> acquireNextSwapchainImageIndex() override {
            unsigned nextImageIndex = 0u;
            auto result = vkAcquireNextImageKHR(
                    device,
                    swapchain->getSwapchain(),
                    acquireImageTimeoutInMs,
                    imageAcquiredSemaphore,
                    VK_NULL_HANDLE, // TODO - figure out what we need to do for synchronization for FIFO
                    &nextImageIndex
            );
            if (result != VK_SUCCESS) {
                // TODO - log!
                return boost::none;
            }
            return {nextImageIndex};
        }

        bool presentSwapchainImage(unsigned int swapchainImageIndex) override {
            VkPresentInfoKHR presentInfo{
                    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    nullptr,
                    1,
                    &renderCompleteSemaphore,
                    1,
                    &swapchain->getSwapchain(),
                    &swapchainImageIndex,
                    nullptr
            };
            return vkQueuePresentKHR(presentQueue, &presentInfo) == VK_SUCCESS;
        }

    private:
        VkDevice device;

        std::shared_ptr<appContext::osInterface::swapchain::vulkan::VulkanSwapchain> swapchain;

        uint64_t acquireImageTimeoutInMs;

        // so it seems like we'll need to have this guy hold onto some synchronization primitives;
        // I can probably make them more robust and efficient later but we basically need to have a
        // semaphore or a fence to signal when an image is available;
        // maybe I could even get fancy now and associate a semaphore with each swapchain image?
        // not sure tho; keeping it simple to start seems the best as usual

        // okay so one scheme I've found for synchronization works like this:
        // 1. we use a semaphore to signal when an image has been acquired;
        // 2. the image acquire semaphore is given as a *wait* semaphore for the graphics queue submission
        // 3. we use another semaphore to signal when rendering to an acquired image has completed;
        //    this "render complete semaphore" is used as a *signal* semaphore for the graphics queue submission (in addition to the image acquire as a wait semaphore)
        // 4. we pass the render complete semaphore as a wait semaphore for the image presentation

        // I think (?) it would make sense to have semaphores for each swapchain image, but I'm not entirely sure...
        // on the one hand, it might be reasonable to just design it around having a single frame in-flight at a time (probably how it should be anyway)
        // we could have a single "rendering complete" semaphore that exists somewhere or other, maybe here would make sense
        // but it could probably go in the renderer too.

        // so if we had the render complete semaphore in the renderer itself, there could be image acquire semaphores for
        // each swapchain image in this class (maybe in a struct or something?)

        VkSemaphore imageAcquiredSemaphore;
        VkSemaphore renderCompleteSemaphore;

        VkQueue presentQueue;
    };
}