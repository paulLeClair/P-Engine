#pragma once

// vulkan includes
#include "../../../vulkan/vulkan_functions/VulkanFunctions.hpp" // i think you just include this...?

// use macro to load vkGetInstanceProcAddr according to OS
#ifdef _WIN32
#define LoadFunction GetProcAddress
#define UNICODE 1
#include <windows.h>
#endif // extend this to add more OSes that support Vulkan

#include <functional>
#include <mutex>
#include <memory>

class PEngine;

namespace Backend {

struct VulkanQueueInfo {
    uint32_t familyIndex; // index of given family
    std::vector<float> priorities;
};

// this should wrap up everything needed before you can create any pipelines etc
struct VulkanInstanceData {

    // any OS-specific data
    #ifdef _WIN32
    // store a reference to the vulkan HMODULE 
    HMODULE vulkanLib;

    #endif
    // any other OSes...

    // vulkan instances
    VkInstance vulkanInstance;

    // vulkan logical device stuff
    VkPhysicalDeviceFeatures desiredDeviceFeatures;
    VkPhysicalDeviceProperties desiredDeviceProperties;

    // physical devices 
        // TODO - expand this to allow for multiple devices eventually
    std::vector<VkPhysicalDevice> physicalDevices;
    VkPhysicalDevice selectedPhysicalDevice = VK_NULL_HANDLE;

    // logical device (i think there should only be one?)
    VkDevice device;

    // queue infos  
    // TODO - make this more configurable
    std::vector<VulkanQueueInfo> queueFamilyInfos;

    // use a single compute queue and a single graphics queue
    VkQueue graphicsQueue;
    VkQueue computeQueue;

    // queue family indices
    uint32_t graphicsQueueFamilyIndex;
    uint32_t computeQueueFamilyIndex;

    // EXTENSIONS
    std::vector<const char*> enabledInstanceExtensionNames;
    std::vector<const char*> enabledDeviceExtensionNames;

    // present/swapchain configuration 
    VkPresentModeKHR presentMode;
    VkSurfaceKHR presentationSurface;
    VkSurfaceTransformFlagsKHR desiredSurfaceTransform;
    VkSurfaceTransformFlagsKHR surfaceTransform;
    VkSurfaceFormatKHR desiredSurfaceFormat;
    VkSurfaceFormatKHR surfaceFormat;

    uint32_t numSwapchainImages;
    VkImageUsageFlags desiredSwapchainImageUsages;
    VkImageUsageFlags swapchainImageUsages;
    VkExtent2D swapchainImageSize;

    ~VulkanInstanceData() {
        // destroy whatever needs destroying
        vkDestroySurfaceKHR(vulkanInstance, presentationSurface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(vulkanInstance, nullptr);
    }
};

class WindowSystem;

class Context : public std::enable_shared_from_this<Context> {
  public:
    Context(PEngine *core);
    ~Context();

    // interface
    void immediateSubmitCommand(std::function<void(VkCommandBuffer)> command);

    #pragma region CONTEXT_GETTERS
    WindowSystem &WSI();

    const VkInstance &getInstance() const {
      return vulkanData_->vulkanInstance;
    }

    const VkDevice &getLogicalDevice() const {
      return vulkanData_->device;
    }

    const VkPhysicalDevice &getSelectedPhysicalDevice() const {
      return vulkanData_->selectedPhysicalDevice;
    }

    const unsigned int &getGraphicsQueueFamilyIndex() const {
      return vulkanData_->graphicsQueueFamilyIndex;
    }

    VkQueue &getGraphicsQueue() {
      return vulkanData_->graphicsQueue;
    }

    VkSurfaceKHR &getPresentationSurface() {
      return vulkanData_->presentationSurface;
    }

    const unsigned int &getMinImageCount();

    const unsigned int getSwapchainImageCount();

    const VkFormat &getSwapchainImageFormat();

    const uint32_t &getSwapchainImageIndex();

    const VkExtent2D &getSwapchainImageSize();

    const VkClearValue &getClearValue();

    const VkImage &getSwapchainImage(unsigned int index);

    const VkImageView &getSwapchainImageView(uint32_t index);

    const VkSwapchainKHR &getSwapchain();

    const uint32_t *getSwapchainImageIndices();

    VkSemaphore &getSwapchainPresentSemaphore();

    VkSemaphore &getSwapchainRenderCompleteSemaphore();

    const unsigned int getCurrentSwapchainImageIndex() const;

    #pragma endregion CONTEXT_GETTERS

  private:
    
    PEngine *core_ = nullptr;
  
    // should probably rethink this monolithic vulkan data structure but i guess it works for now
    std::shared_ptr<VulkanInstanceData> vulkanData_;

    // the render context can also hold onto the window system integration
    std::shared_ptr<WindowSystem> wsi_;

    /* rendering  */
        // actually a lot of this should go in the frame context
    std::mutex commandBuffersLock_;
    // std::vector<VkCommandBuffer> recordedBuffers_;
    VkFence frameFence_;

    // immediate submission stuff
    std::mutex immediateSubmitLock;
    VkCommandPool coreCommandPool_;
    VkFence coreCommandPoolFence_;

    // startup / setup!

    // create vulkan instance
    void initialVulkanSetup(VulkanInstanceData &instanceToFill); 
    
    void createVulkanInstance(VulkanInstanceData &instanceToFill);

    // preliminary swapchain/presentation stuff (before the WSI starts handling everything)
    void createPresentationSurface(VulkanInstanceData &instance);
    void setPresentationMode(VulkanInstanceData &instance);
    void setSwapchainImageUses(VulkanInstanceData &instance, VkSurfaceCapabilitiesKHR &capabilities);
    void setSwapchainImageFormat(VulkanInstanceData &instance, const VkPhysicalDevice &device);
    void setSwapchainImageTransform(VulkanInstanceData &instance, VkSurfaceCapabilitiesKHR &capabilities);

    // load instance extensions
    void loadInstanceExtensions(VulkanInstanceData &instance);
        bool checkInstanceExtensionAvailability(const char *instanceExtensionNameToCheck, const std::vector<VkExtensionProperties> &availableExtensions);
        void setRequiredInstanceExtensions(std::vector<const char *> &nameVectorToFill);
    
    // load instance-level entry points
    void loadInstanceLevelEntryPoints(VulkanInstanceData &instanceToFill);

    // create logical device
    void createLogicalDevice(VulkanInstanceData &instance);
        // select queue family indices
        bool deviceLimitCheck(VkPhysicalDeviceLimits &limits);
        bool deviceExtensionsCheck(VulkanInstanceData &instance, const VkPhysicalDevice &device);
        void setRequiredDeviceExtensions(std::vector<const char *> &nameVectorToFill);
        void choosePhysicalDevice(VulkanInstanceData &instance, std::vector<VkPhysicalDevice> &physicalDevices, VkPhysicalDeviceFeatures &features);
        void selectQueueFamilyIndices(VulkanInstanceData &instance, const VkPhysicalDevice &device, bool compute); // just gonna add a toggle boolean so i dont have 2 functions
        bool checkSurfacePresentCapabilities(VulkanInstanceData &instance, const VkPhysicalDevice &device);
        std::vector<VkDeviceQueueCreateInfo> getQueueCreateInfos(VulkanInstanceData &instance);

    // load device-level entry points
    void loadDeviceLevelEntryPoints(VulkanInstanceData &instancetoFill);
    
};

} // namespace Backend