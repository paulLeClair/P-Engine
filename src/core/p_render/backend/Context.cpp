#include "../../../../include/core/p_render/backend/Context.hpp"

#include "../../../../include/core/p_render/backend/wsi/WindowSystem.hpp"

#include "../../../../include/core/PEngineCore.hpp"

#include "../../../../include/core/p_render/PRender.hpp"

#include "../../../../include/core/PEngineCore.hpp"

using namespace Backend;

Context::Context(PEngine *core) : core_(core) {
    // should try to follow RAII when possible -> allocate/create/etc in the ctor, deallocate/destroy/etc in the dtor 
    vulkanData_ = std::make_shared<VulkanInstanceData>();

    initialVulkanSetup(*vulkanData_);

    // set up the frame fence
    VkFenceCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = 0;
    info.pNext = nullptr;

    auto result = vkCreateFence(vulkanData_->device, &info, nullptr, &frameFence_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create frame fence!");
    } 

    result = vkCreateFence(vulkanData_->device, &info, nullptr, &coreCommandPoolFence_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create core command pool fence!");
    }
    
    wsi_ = std::make_shared<WindowSystem>(core_, this);
}

Context::~Context() {

}

// swapchain information getters
const unsigned int &Context::getMinImageCount() {
      return wsi_->getMinImageCount(); 
    }

const unsigned int Context::getSwapchainImageCount() {
    return wsi_->getSwapchainImageCount();
}

const VkFormat &Context::getSwapchainImageFormat() {
    return wsi_->getSwapchainImageFormat();
}

const uint32_t &Context::getSwapchainImageIndex() {
    return wsi_->getSwapchainImageIndex(); 
}

const VkExtent2D &Context::getSwapchainImageSize() {
    return wsi_->getSwapchainImageSize();
}

const VkClearValue &Context::getClearValue() {
    return wsi_->getClearValue();
}

const VkImageView &Context::getSwapchainImageView(uint32_t index) {
    return wsi_->getSwapchainImageView(index);
}

const VkImage &Context::getSwapchainImage(unsigned int index) {
    return wsi_->getSwapchainImage(index);
}

const VkSwapchainKHR &Context::getSwapchain() {
    return wsi_->getSwapchain();
}

const uint32_t *Context::getSwapchainImageIndices() {
    return wsi_->getSwapchainImageIndices();
}

WindowSystem &Context::WSI() {
      return *wsi_;
}

VkSemaphore &Context::getSwapchainPresentSemaphore() {
        return wsi_->getSwapchainPresentSemaphore();
}

VkSemaphore &Context::getSwapchainRenderCompleteSemaphore() {
    return wsi_->getSwapchainRenderCompleteSemaphore();
}

const unsigned int Context::getCurrentSwapchainImageIndex() const {
    return wsi_->getCurrentSwapchainImageIndex();
}

void Context::initialVulkanSetup(VulkanInstanceData &instance) {
    /* PHASE 1 */
    // load global and exported entry points
        // not entirely sure what exported entry points are for
    // TODO: proper error handling
    #ifdef _WIN32

    auto vulkanLibraryModule = core_->getVulkanLibraryModule();
    #define EXPORTED_VULKAN_FUNCTION( name ) \
    name = (PFN_##name)LoadFunction(vulkanLibraryModule, #name);\
    if (name == nullptr) {\
        std::cout << "oh no! couldn't load exported function" << std::endl;\
        exit(1);\
    }

    #include "../../../../include/vulkan/vulkan_functions/VulkanFunctionsList.inl"

    #define GLOBAL_LEVEL_VULKAN_FUNCTION( name )        \
    name = (PFN_##name)vkGetInstanceProcAddr(nullptr, #name);       \
    if (name == nullptr) { \
            std::cout << "oh no! couldn't load global function" << std::endl;\
            exit(1); \
    }

    #include "../../../../include/vulkan/vulkan_functions/VulkanFunctionsList.inl"

    // load instance-level vulkan extensions
    loadInstanceExtensions(instance);

    // create the Vulkan instance
    createVulkanInstance(instance);

    const auto &setupValidationLayers = [&](VulkanInstanceData &instance) {
        
    };

    if (true) {
        setupValidationLayers(instance);
    }

    // load instance-level entry points
    loadInstanceLevelEntryPoints(instance);

    // create logical device
    createLogicalDevice(instance);

    // load device-level entry points
    loadDeviceLevelEntryPoints(instance);

    // get graphics/compute queues
    vkGetDeviceQueue(instance.device, instance.graphicsQueueFamilyIndex, 0, &instance.graphicsQueue);
    vkGetDeviceQueue(instance.device, instance.computeQueueFamilyIndex, 0, &instance.computeQueue);

    // setup core command pool for immediate submissions
    VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.pNext = nullptr;
    poolInfo.queueFamilyIndex = vulkanData_->graphicsQueueFamilyIndex; // for now we just worry about unified graphics+compute
    auto result = vkCreateCommandPool(vulkanData_->device, &poolInfo, nullptr, &coreCommandPool_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create core command pool");
    }

    #endif 
    
}

void Context::choosePhysicalDevice(VulkanInstanceData &instance, std::vector<VkPhysicalDevice> &physicalDevices, VkPhysicalDeviceFeatures &features) {
    // loop over all physical devices
    // first set required device extensions
    setRequiredDeviceExtensions(instance.enabledDeviceExtensionNames);
    for (const auto &physicalDevice : physicalDevices) {
        // begin checking the physical device characteristics:

        /* CHECK DEVICE PROPERTIES */
        // get physical device properties
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        // continue if physical device is anything but a discrete GPU
            // for now, only gonna try to run on graphics cards
        if (physicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            continue;
        }

        // i suppose i should make a function that just wraps up all device limit checks... for now i'm not gonna make it too fancy
        if (!deviceLimitCheck(physicalDeviceProperties.limits)) {
            continue;
        }

        /* CHECK DEVICE FEATURES */
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);
        
        // only features i need to check rn:
            // tessellation shader
        if (!features.tessellationShader) {
            continue;
        }
        // whatever other features need checking...

        // finally just unset all other features, or else they will be used i think
        features = {};
        
        // tessellation shader;
        features.tessellationShader = VK_TRUE;

        /* CHECK DEVICE EXTENSIONS */
        if (!deviceExtensionsCheck(instance, physicalDevice)) {
            continue;
        }

        // not sure if any other checks need to be made...
        instance.selectedPhysicalDevice = physicalDevice;
    }

    // if we haven't chosen a physical device
    if (instance.selectedPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("unable to find suitable graphics and/or compute device!");
    }
    else {
        // set up anything that should be set after the physical device has been chosen, for now just queue family indices
        selectQueueFamilyIndices(instance, instance.selectedPhysicalDevice, false); // false to set graphics queue family index 
        selectQueueFamilyIndices(instance, instance.selectedPhysicalDevice, true); // true to set compute queue family index
    }
}

bool Context::checkSurfacePresentCapabilities(VulkanInstanceData &instance, const VkPhysicalDevice &device) {

    VkSurfaceCapabilitiesKHR capabilities;
    auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, instance.presentationSurface, &capabilities);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to get physical device surface capabilities!");
        exit(1);
    }

    instance.numSwapchainImages = (instance.presentMode == VK_PRESENT_MODE_MAILBOX_KHR) ? 3 : capabilities.minImageCount + 1; // either triple buffer or just use minimagecount...
    if (capabilities.maxImageCount > 0) {
        if (instance.numSwapchainImages > capabilities.maxImageCount) {
            instance.numSwapchainImages = capabilities.maxImageCount;
        }
    }

    if ((capabilities.currentExtent.width == 0xFFFFFFFF) || (capabilities.currentExtent.height == 0xFFFFFFFF)) {
        instance.swapchainImageSize.width = std::clamp<uint32_t>(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        instance.swapchainImageSize.height = std::clamp<uint32_t>(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    else {
        instance.swapchainImageSize = capabilities.currentExtent;
    }
    
    // check/set desired swapchain IMAGE USAGES for this surface
    setSwapchainImageUses(instance, capabilities);

    // check/set TRANSFORM
    setSwapchainImageTransform(instance, capabilities);

    // check/set FORMAT
    setSwapchainImageFormat(instance, device);

    return true;
}

void Context::setSwapchainImageUses(VulkanInstanceData &instance, VkSurfaceCapabilitiesKHR &capabilities) {
    instance.desiredSwapchainImageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // now have to check against the possible image usages
    VkImageUsageFlags potentialImageUsages = instance.desiredSwapchainImageUsages & capabilities.supportedUsageFlags;
    if (potentialImageUsages != instance.desiredSwapchainImageUsages) {
        throw std::runtime_error("Desired swapchain image usage not available!");
    }
    else {
        instance.swapchainImageUsages = instance.desiredSwapchainImageUsages;
    }
}

void Context::setSwapchainImageTransform(VulkanInstanceData &instance, VkSurfaceCapabilitiesKHR &capabilities) {
    instance.desiredSurfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    if (instance.desiredSurfaceTransform & capabilities.supportedTransforms) {
        instance.surfaceTransform = instance.desiredSurfaceTransform;
    }
    else {
        throw std::runtime_error("Desired surface transform unavailable!"); // shouldn't happen
    }
}

void Context::setSwapchainImageFormat(VulkanInstanceData &instance, const VkPhysicalDevice &device) {
    // set the swapchain image format here
        // not entirely sure what needs to be done, gotta read

    // i think we're setting 3 things:
        // 1. VkFormat -> image format (encodes stuff like the components, precision, data type for the pixels of an image/formatted buffer)
        // 2. VkColorSpaceKHR -> choose the color space for the engine (how colors are encoded for hardware)
        // 3. VkSurfaceFormatKHR -> a pairing of 1 and 2 
    
    /* SET DESIRED FORMAT */
    // gonna try these for now 
    // instance.desiredSurfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM; //FEB17- found the problem maybe: incorrect surface format!
    instance.desiredSurfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
    instance.desiredSurfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    // get formats using selected physical device
    uint32_t formatsCount;
    auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, instance.presentationSurface, &formatsCount, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("unable to acquire physical device surface formats!");
    }

    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, instance.presentationSurface, &formatsCount, surfaceFormats.data()); 
    if (result != VK_SUCCESS) {
        throw std::runtime_error("unable to acquire physical device surface formats!");
    }

    // first check if surfaceFormats has only one element equal to VK_FORMAT_UNDEFINED, which means we can use anything for the surface format
    if ((surfaceFormats.size() == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        // set the format as we want?
        instance.surfaceFormat = instance.desiredSurfaceFormat;
        return;
    }

    // else we have to set the format this way:
    bool foundFormat = false;
    for (const auto &format : surfaceFormats) {
        if ((format.format == instance.desiredSurfaceFormat.format) && (format.colorSpace == instance.desiredSurfaceFormat.colorSpace)) {
            instance.surfaceFormat.format = instance.desiredSurfaceFormat.format;
            instance.surfaceFormat.colorSpace = instance.desiredSurfaceFormat.colorSpace;
            foundFormat = true; 
            break;
        }
    }

    if (!foundFormat) {
        // just set it to tthe first available format...
        instance.surfaceFormat.format = surfaceFormats[0].format;
        instance.surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
    }
}

// load instance extensions
void Context::loadInstanceExtensions(VulkanInstanceData &instance) {
    // first set the instance level extensions that are required? i'll just do this in a function for now
    setRequiredInstanceExtensions(instance.enabledInstanceExtensionNames);

    // second get the extension capabilities of the system
    uint32_t num_extensions = 0;
    if ((vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr)) != VK_SUCCESS) {
        // uh oh!
        throw std::runtime_error("Unable to enumerate instance extension properties!");
    }

    // quick size check
    if (num_extensions < instance.enabledInstanceExtensionNames.size()) {
        throw std::runtime_error("Not all instance extensions are supported!");
    }

    // acquire available extensions info
    std::vector<VkExtensionProperties> availableExtensions(num_extensions);
    if ((vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, availableExtensions.data())) != VK_SUCCESS) {
        throw std::runtime_error("Unable to enumerate instance extension properties!");
    }

    // check that all extensions are available
    for (size_t i = 0; i < instance.enabledInstanceExtensionNames.size(); i++) {
        if (!checkInstanceExtensionAvailability(instance.enabledInstanceExtensionNames[i], availableExtensions)) {
            throw std::runtime_error("Unsupported instance-level extension: " + std::string(instance.enabledInstanceExtensionNames[i]));
        }
    }
}

bool Context::checkInstanceExtensionAvailability(const char *instanceExtensionName, const std::vector<VkExtensionProperties> &availableExtensions) {
    // this is where we check the availability of a given instance extension by name 
    std::string str_ext_name(instanceExtensionName);

    for (size_t i = 0; i < availableExtensions.size(); i++) {
        std::string str_avail_name(availableExtensions[i].extensionName);
        if (str_ext_name == str_avail_name) {
            return true;
        }
    }
    return false;
}

void Context::setRequiredInstanceExtensions(std::vector<const char *> &nameVectorToFill) {
    // for now just need these extensions i think
    nameVectorToFill.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    nameVectorToFill.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

void Context::createVulkanInstance(VulkanInstanceData &instance) {
    // create vulkan instance
        // should only be called after instance extension names were fixed
     // prepare app info struct for Vulkan (make this more specifiable as we go, for now it'll just use a satisfactory config)
    VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "PGAME2020",
        VK_MAKE_VERSION(1, 2, 141), 
        "PEngine2020",
        VK_MAKE_VERSION(1, 2, 141),
        VK_MAKE_VERSION(1, 2, 141)
    };

    // hardcoding validation layers until further notice
    std::vector<const char *> enabledLayers = {};
    if (true) {
        // here we could replace the "true" with a "enableLayers" condition and add the layers only in this case
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
    }    

    VkInstanceCreateInfo instance_create_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &app_info,
        static_cast<unsigned int>(enabledLayers.size()),
        (enabledLayers.size()) ? enabledLayers.data() : nullptr,
        static_cast<uint32_t>(instance.enabledInstanceExtensionNames.size()),
        instance.enabledInstanceExtensionNames.data(),

    };

    if (vkCreateInstance(&instance_create_info, nullptr, &instance.vulkanInstance) != VK_SUCCESS) { //testies
        // could not create vulkan instance :(
        throw std::runtime_error("Unable to create vulkan instance!");
    }

}

// load instance-level entry points
void Context::loadInstanceLevelEntryPoints(VulkanInstanceData &instance) {
    #define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) \
    name = (PFN_##name)vkGetInstanceProcAddr(instance.vulkanInstance, #name); \
    if (name == nullptr) { \
        exit(1); \
    }

    #include "../../../../include/vulkan/vulkan_functions/VulkanFunctionsList.inl"
}

void Context::createLogicalDevice(VulkanInstanceData &instance) {

    // first get only the NUMBER of physical devices 
    uint32_t numPhysicalDevices = 0;
    auto result = vkEnumeratePhysicalDevices(instance.vulkanInstance, &numPhysicalDevices, nullptr);
    if ((result != VK_SUCCESS) || (numPhysicalDevices == 0)) {
        throw std::runtime_error("Unable to enumerate physical devices!");
    }

    // then actually acquire the number of physical devices that we found
    std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
    result = vkEnumeratePhysicalDevices(instance.vulkanInstance, &numPhysicalDevices, physicalDevices.data());
    if ((result != VK_SUCCESS) || (numPhysicalDevices == 0)) {
        throw std::runtime_error("Unable to enumerate physical devices!");
    }

    // create presentation surface
    createPresentationSurface(instance);

    // choose physical device for graphics and compute
    VkPhysicalDeviceFeatures physDeviceFeatures;
    choosePhysicalDevice(instance, physicalDevices, physDeviceFeatures);

    // i think here we have to check presentation modes, no matter what FIFO is supported though so at the end of this section
    // the surface present mode should either be mailbox or fifo
    setPresentationMode(instance); 

    // need just single compute and graphics queues for now...
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = getQueueCreateInfos(instance);

    // now i think we can create the logical device...
    VkDeviceCreateInfo logicalDeviceCreateInfo;
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pNext = nullptr;
    logicalDeviceCreateInfo.flags = 0;
    logicalDeviceCreateInfo.enabledLayerCount = 0;
    logicalDeviceCreateInfo.ppEnabledLayerNames = nullptr;
    logicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    logicalDeviceCreateInfo.pQueueCreateInfos = (queueCreateInfos.size() > 0) ? queueCreateInfos.data() : nullptr;
    logicalDeviceCreateInfo.pEnabledFeatures = &physDeviceFeatures;

    // set up device extensions (think i was missing this?)
    logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instance.enabledDeviceExtensionNames.size());
    logicalDeviceCreateInfo.ppEnabledExtensionNames = instance.enabledDeviceExtensionNames.data();

    // create device!
    result = vkCreateDevice(instance.selectedPhysicalDevice, &logicalDeviceCreateInfo, nullptr, &instance.device);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to create logical device!");
    }
}

void Context::setPresentationMode(VulkanInstanceData &instance) {
    uint32_t presentationModesCount = 0;
    VkResult result = VK_SUCCESS;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(instance.selectedPhysicalDevice, instance.presentationSurface, &presentationModesCount, nullptr);
    if ((result != VK_SUCCESS) || (presentationModesCount == 0)) {
        throw std::runtime_error("Unable to get physical device surface present modes!");
    }

    std::vector<VkPresentModeKHR> presentationModes(presentationModesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(instance.selectedPhysicalDevice, instance.presentationSurface, &presentationModesCount, presentationModes.data());
    if ((result != VK_SUCCESS) || (presentationModesCount == 0)) {
        throw std::runtime_error("Unable to get physical device surface present modes!");
    }

    bool mailboxFound = false;
    for (const auto &presentMode : presentationModes) {
        // look for mailbox present mode
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            mailboxFound = true;
            break;
        }
    }

    // set the present mode 
    if (mailboxFound) {
        // set it to MAILBOX
        instance.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else {
        instance.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
}

void Context::createPresentationSurface(VulkanInstanceData &instance) {

    #ifdef _WIN32

    struct WindowData {
        HINSTANCE hinst;
        HWND hwnd;
    } windowData;

    windowData.hinst = core_->getHINSTANCE();
    windowData.hwnd = core_->getMainWindowHWND();

    // create win32 surface 
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        nullptr,
        0,
        windowData.hinst,
        windowData.hwnd
    };
    win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfo.pNext = nullptr;
    win32SurfaceCreateInfo.flags = 0;
    win32SurfaceCreateInfo.hinstance = windowData.hinst;
    win32SurfaceCreateInfo.hwnd = windowData.hwnd; 

    instance.presentationSurface = VK_NULL_HANDLE;
    if (vkCreateWin32SurfaceKHR(instance.vulkanInstance, &win32SurfaceCreateInfo, nullptr, &instance.presentationSurface) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create presentation surface!");
    }

    if (instance.presentationSurface == VK_NULL_HANDLE) {
        throw std::runtime_error("Unable to create presentation surface!");
    }
    #endif
}

// get queue create infos
std::vector<VkDeviceQueueCreateInfo> Context::getQueueCreateInfos(VulkanInstanceData &instance) {
    VulkanQueueInfo graphicsQueueInfo;
    graphicsQueueInfo.familyIndex = instance.graphicsQueueFamilyIndex;
    graphicsQueueInfo.priorities.clear(); graphicsQueueInfo.priorities.push_back(1.0);
    instance.queueFamilyInfos.push_back(graphicsQueueInfo);

    VulkanQueueInfo computeQueueInfo;
    computeQueueInfo.familyIndex = instance.computeQueueFamilyIndex;
    computeQueueInfo.priorities.clear(); computeQueueInfo.priorities.push_back(1.0);
    instance.queueFamilyInfos.push_back(computeQueueInfo);

    // build queuecreateinfo struct?
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (auto &info : instance.queueFamilyInfos) {
        // create queue
        VkDeviceQueueCreateInfo temp;
        temp.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        temp.pNext = nullptr;
        temp.flags = 0;
        temp.queueFamilyIndex = info.familyIndex;
        temp.queueCount = static_cast<uint32_t>(info.priorities.size());
        temp.pQueuePriorities = (info.priorities.size() > 0) ? info.priorities.data() : nullptr;

        queueCreateInfos.push_back(temp);
    }

    return queueCreateInfos;
}

// device limit check
bool Context::deviceLimitCheck(VkPhysicalDeviceLimits &limits) {
    // TODO - actually check the device
    return true;
}

// device extension check
bool Context::deviceExtensionsCheck(VulkanInstanceData &instance, const VkPhysicalDevice &device) {
    uint32_t extensionsCount = 0;
    auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
    if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
        return false;
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());
    if ((result != VK_SUCCESS) || (extensionsCount == 0)) {
        return false;
    }

    // check all required extensions
    std::vector<const char *> foundExtensions;
    for (const auto &requiredExtensionName : instance.enabledDeviceExtensionNames) {
        std::string requiredExtensionNameStr(requiredExtensionName);
        for (const auto &extension : availableExtensions) {
            std::string extensionName(extension.extensionName);
            if (requiredExtensionNameStr == extensionName) {
                foundExtensions.push_back(extension.extensionName);
            }
        }
    }

    // if all req extensions are present, return true
    if (foundExtensions.size() == instance.enabledDeviceExtensionNames.size()) {
        return true;
    }
    else {
        return false;
    }   
}

void Context::setRequiredDeviceExtensions(std::vector<const char *> &nameVectorToFill) {
    nameVectorToFill.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void Context::loadDeviceLevelEntryPoints(VulkanInstanceData &instance) {
    #define DEVICE_LEVEL_VULKAN_FUNCTION( name )                \
    name = (PFN_##name)vkGetDeviceProcAddr(instance.device, #name);     \
    if (name == nullptr) {                                      \
        exit(1); \
    }

    #include "../../../../include/vulkan/vulkan_functions/VulkanFunctionsList.inl"
}

// select queue family indices
void Context::selectQueueFamilyIndices(VulkanInstanceData &instance, const VkPhysicalDevice &device, bool compute) {
    // first get the number of queuefamilyproperties or whteva ;)
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0) {
        throw std::runtime_error("Unable to get physical device queue family properties!");
    }

    // fill queue family properties vector
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, queueFamilies.data());
    if (queueFamilyPropertyCount == 0) {
        throw std::runtime_error("Unable to get physical device queue family properties!");
    }

    if (!compute) {
        // look for a suitable graphics queue family
        // i think this just involves checking that presentation to our surface is supported
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
            VkBool32 presentationSupported = VK_FALSE;
            auto result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, instance.presentationSurface, &presentationSupported);

            if (result != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface support!");
            }

            if ((presentationSupported == VK_TRUE) && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                instance.graphicsQueueFamilyIndex = i;
                break; // move on once we find one
            }
        }
    }
    else {
        // look for suitable compute queue family
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilies.size()); i++) {
            // not sure i guess just select whichever queue supports compute ???
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                instance.computeQueueFamilyIndex = i;
                break;
            }
        }
    }
    
    // TODO: check if this found a queue family    
}

/* RENDERING */
void Context::immediateSubmitCommand(std::function<void(VkCommandBuffer)> command) {
    // this function should basically submit a std::function-wrapped command to vulkan

    // allocate the default command buffer that will be used for instant commands
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = coreCommandPool_;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    auto result = vkAllocateCommandBuffers(vulkanData_->device, &allocInfo, &cmd);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to allocate command buffer from base command pool!");
    }

    // create begin info for the command
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.pNext = nullptr;

    // begin recording the command buffer so we can submit it 
    result = vkBeginCommandBuffer(cmd, &cmdBeginInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to begin command buffer!");
    }

    // execute the submitted command, providing it cmd as its VkCommandBuffer argument
    command(cmd);

    result = vkEndCommandBuffer(cmd);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to end command buffer!");
    }

    // now that the command buffer has been recorded, we can submit it and wait for it to complete
    VkSubmitInfo submitInfo = {};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    
    result = vkQueueSubmit(vulkanData_->graphicsQueue, 1u, &submitInfo, coreCommandPoolFence_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Unable to submit to core command pool!");
    }

    // now wait on the fence, which i think should only be used for these immediate submits 
    vkWaitForFences(vulkanData_->device, 1, &coreCommandPoolFence_, true, 9999999); 
    vkResetFences(vulkanData_->device, 1, &coreCommandPoolFence_);

    // clear the main command pool (means you better not have other buffers you wanna use here!)
    vkResetCommandPool(vulkanData_->device, coreCommandPool_, 0);
}