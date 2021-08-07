// not at all sure where/how this has to be positioned so it works the way i need it to
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR // expand this for other OSes eventually
#include "../vulkan.h"
    // FEB24 - disabling this to try something
        // doesnt seem to work to disable that

// use a namespace for the wrapper

// exported
#define EXPORTED_VULKAN_FUNCTION( name ) extern PFN_##name name;

// global 
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name;

// instance
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name;

// instance level from extension
// #define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name;

// device 
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) extern PFN_##name name;

// device level from extension
// #define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name;

// include the inline file
#include "./VulkanFunctionsList.inl"