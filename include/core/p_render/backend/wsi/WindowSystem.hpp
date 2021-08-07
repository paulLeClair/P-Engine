#pragma once

// BACKEND REWRITE 
    // main idea: take the windowing functionality out of the OS interface and incorporate more closely
    // into the renderer; it can also handle the VkSwapchain
#include "../Context.hpp"

#include <vector>

class PEngine;

namespace Backend {

struct WindowSystemData {
    // tbh, this is kinda pointless given that we pass pointers to the core and context; might just make sense
    // to get all the relevant info from there and have these be regular members? idk

    /* WIN32 */
    // i think for now we just support one main window, so i'll hardcode that
    HINSTANCE win32Instance;
    HWND mainWindow;
    HRGN mainWindowRegion;

    /* VULKAN */
    // physical device (think it's only needed for a couple things)
    VkDevice logicalDevice;
    VkPhysicalDevice selectedPhysicalDevice;
    VkQueue presentQueue; // should just be graphics queues for now 

    uint32_t numSwapchainImages; 
    VkExtent2D swapchainImageSize;
    VkImageUsageFlags desiredImageUsages;
    VkImageUsageFlags imageUsages;

    // surface extension stuff
    VkSurfaceTransformFlagBitsKHR desiredSurfaceTransform;
    VkSurfaceTransformFlagBitsKHR surfaceTransform;
    VkSurfaceFormatKHR desiredSurfaceFormat;
    VkSurfaceFormatKHR surfaceFormat;

    // presentation surface
    VkSurfaceKHR presentationSurface;
    VkPresentModeKHR desiredPresentMode;
    VkPresentModeKHR presentMode;
    VkClearValue clearValue = {164.0f/256.0f, 30.0f/256.0f, 34.0f/256.0f, 0.0f}; // rando default

    // swapchain stuff
    VkSwapchainKHR swapchain; // for now we're using a single swapchain, but could make this a vector of swapchains when needed
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<uint32_t> swapchainImageIndices;
    uint32_t currentSwapchainImageIndex; // again could package this all up and make a struct to have multiple swapchains in use

    // for now i'll try just using simple semaphores, maybe i can look into more complicated swapchain synchronization later
    VkSemaphore swapchainRenderCompleteSemaphore;
    VkSemaphore swapchainPresentSemaphore;
    // std::vector<VkSemaphore> swapchainImageAcquiredSemaphores;
    // std::vector<VkSemaphore> swapchainRenderCompleteSemaphores;

};

// forward declarations
class Context;

class WindowSystem {
  public:
    WindowSystem(PEngine *core, Context *context);
    ~WindowSystem() = default;

    // acquire / present swapchain image 
    unsigned int acquireNextSwapchainImage() {
        auto result = vkAcquireNextImageKHR(context_->getLogicalDevice(), wsiData_->swapchain, 20000000, 
                                            wsiData_->swapchainPresentSemaphore, VK_NULL_HANDLE,
                                            &wsiData_->currentSwapchainImageIndex);

        switch (result) {
            case VK_SUCCESS:
            case VK_SUBOPTIMAL_KHR:
                return wsiData_->currentSwapchainImageIndex; 
                break;
            default:
                return -1;
        }
    }

    void presentImage() {
        
        VkPresentInfoKHR info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        info.pNext = nullptr;
        info.swapchainCount = 1;
        info.pSwapchains = &wsiData_->swapchain;
        info.pResults = nullptr;
        info.waitSemaphoreCount = 1; 
        info.pWaitSemaphores = &wsiData_->swapchainRenderCompleteSemaphore; // wait until renderComplete semaphore is signaled 
        info.pImageIndices = wsiData_->swapchainImageIndices.data();
        
        auto result = vkQueuePresentKHR(context_->getGraphicsQueue(), &info); // for now graphics queue is the present queue always
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Unable to present image!");
        }
        
    }

    #pragma region WSI_GETTERS
    WindowSystemData &getWindowSystemData() const {
      return *wsiData_;
    }

    const unsigned int &getMinImageCount() const {
      return wsiData_->numSwapchainImages; // gonna try this, not sure how much of an impact it has
    }

    unsigned int getSwapchainImageCount() {
      return wsiData_->numSwapchainImages;
    }

    const VkFormat &getSwapchainImageFormat() const {
      return wsiData_->surfaceFormat.format;
    }

    const uint32_t &getSwapchainImageIndex() const {
      return wsiData_->currentSwapchainImageIndex; // hopefully this doesnt cause any issues
    }

    const VkExtent2D &getSwapchainImageSize() const {
      return wsiData_->swapchainImageSize;
    }

    const VkClearValue &getClearValue() const {
      return wsiData_->clearValue;
    }

    // const VkImageView &getSwapchainImageView(uint32_t index) const {
    //   return wsiData_->swapchainImageViews[index];
    // }

    const VkImage &getSwapchainImage(unsigned int index) {
      if (index >= wsiData_->numSwapchainImages)
        throw std::runtime_error("Invalid swapchain index!");
      
      return wsiData_->swapchainImages[index];
    }

    const VkSwapchainKHR &getSwapchain() const {
      return wsiData_->swapchain;
    }

    const uint32_t *getSwapchainImageIndices() const {
      return wsiData_->swapchainImageIndices.data();
    }

    VkSemaphore &getSwapchainPresentSemaphore() const {
      return wsiData_->swapchainPresentSemaphore;
    }

    VkSemaphore &getSwapchainRenderCompleteSemaphore() const {
      return wsiData_->swapchainRenderCompleteSemaphore;
    }

    const unsigned int getCurrentSwapchainImageIndex() const {
      return wsiData_->currentSwapchainImageIndex;
    }

    VkImageView &getSwapchainImageView(unsigned int index) {
      return wsiData_->swapchainImageViews[index];
    }
    #pragma endregion WSI_GETTERS

    // RANDOM UTILS
    bool checkSurfacePresentCapabilities(const VkPhysicalDevice &device); // used only during context creation

  private:
    PEngine *core_;

    Context *context_;

    std::unique_ptr<WindowSystemData> wsiData_;

    #ifdef _WIN32


    #endif

    // gonna try and move the swapchain-related stuff into this WSI class 
    // these should all just be startup things
    void createPresentationSurface(WindowSystemData &wsiData);

    void setPresentationMode(WindowSystemData &wsiData);
    

    void setupSwapchain(WindowSystemData &wsiData);
        void setSurfacePresentCapabilities(WindowSystemData &wsiData); 
        void getSwapchainImageHandles(WindowSystemData &wsiData);
        void setSwapchainImageUses(WindowSystemData &wsiData, VkSurfaceCapabilitiesKHR &capabilities);
        void setSwapchainImageTransform(WindowSystemData &wsiData, VkSurfaceCapabilitiesKHR &capabilities);
        void setSwapchainImageFormat(WindowSystemData &wsiData);
        void createSwapchainImageSemaphores(WindowSystemData &wsiData);
        void getSwapchainImageViews(WindowSystemData &wsiData);
    
    // 

    

};

} // Backend