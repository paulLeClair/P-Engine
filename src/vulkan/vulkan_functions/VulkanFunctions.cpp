#include "../../../include/vulkan/vulkan_functions/VulkanFunctions.hpp"

// exported
#define EXPORTED_VULKAN_FUNCTION( name ) PFN_##name name;

// global 
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name;

// instance
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name;

// device 
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name;                                                                                                              

// include the inline file
#include "../../../include/vulkan/vulkan_functions/VulkanFunctionsList.inl"
