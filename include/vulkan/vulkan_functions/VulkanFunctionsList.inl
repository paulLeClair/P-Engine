// inline file for wrapping/dynamically loading 
// vulkan functions
    // apparently there's a performance benefit and 
    // it gives you more control over what's happening 


// DECEMBER 2020 -
    // this is all being directly taken from pgame2020, so it may need adjustment
    // i need to learn quickly how to add vulkan to a cmake project

// macros for different types of vulkan functions:
#ifndef EXPORTED_VULKAN_FUNCTION
#define EXPORTED_VULKAN_FUNCTION(function)
#endif 

// CORE FUNCTION: only one that MUST be publically exported in all vulkan programs
EXPORTED_VULKAN_FUNCTION( vkGetInstanceProcAddr ) // do i need the define here??!?!?!?!?

#undef EXPORTED_VULKAN_FUNCTION

// global functions
#ifndef GLOBAL_LEVEL_VULKAN_FUNCTION
#define GLOBAL_LEVEL_VULKAN_FUNCTION(function)
#endif

// 3 global functionez
GLOBAL_LEVEL_VULKAN_FUNCTION( vkEnumerateInstanceExtensionProperties )
GLOBAL_LEVEL_VULKAN_FUNCTION( vkEnumerateInstanceLayerProperties )
GLOBAL_LEVEL_VULKAN_FUNCTION( vkCreateInstance )

#undef GLOBAL_LEVEL_VULKAN_FUNCTION

// instance level functions
#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION
#define INSTANCE_LEVEL_VULKAN_FUNCTION(function)
#endif

// here are the instance level functions i'll need
INSTANCE_LEVEL_VULKAN_FUNCTION(vkDestroyInstance)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkEnumeratePhysicalDevices)
INSTANCE_LEVEL_VULKAN_FUNCTION(vkGetPhysicalDeviceProperties)
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceFeatures )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceQueueFamilyProperties )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkCreateDevice )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetDeviceProcAddr )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkEnumerateDeviceExtensionProperties )

// here are extension-level instance functions (just use portability macros instead of making a separate category)
INSTANCE_LEVEL_VULKAN_FUNCTION( vkDestroySurfaceKHR )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceSurfaceSupportKHR )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceSurfaceCapabilitiesKHR )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceSurfaceFormatsKHR )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceSurfacePresentModesKHR )
INSTANCE_LEVEL_VULKAN_FUNCTION( vkCreateWin32SurfaceKHR )

INSTANCE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceMemoryProperties ) // is... it this?

#undef INSTANCE_LEVEL_VULKAN_FUNCTION

// do i need instance level functions from extension???? i don't think so 

// device level functions
#ifndef DEVICE_LEVEL_VULKAN_FUNCTION
#define DEVICE_LEVEL_VULKAN_FUNCTION( function )
#endif

DEVICE_LEVEL_VULKAN_FUNCTION( vkGetDeviceQueue )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyDevice )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDeviceWaitIdle )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkGetBufferMemoryRequirements )

// other device funcs list
DEVICE_LEVEL_VULKAN_FUNCTION( vkAllocateMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkBindBufferMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdPipelineBarrier )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkGetImageMemoryRequirements )
DEVICE_LEVEL_VULKAN_FUNCTION( vkBindImageMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateImageView )
DEVICE_LEVEL_VULKAN_FUNCTION( vkMapMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkFlushMappedMemoryRanges )
DEVICE_LEVEL_VULKAN_FUNCTION( vkUnmapMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdCopyBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdCopyBufferToImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdCopyImageToBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkBeginCommandBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkEndCommandBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkQueueSubmit )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyImageView )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkFreeMemory )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateCommandPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkAllocateCommandBuffers )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateSemaphore )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateFence )
DEVICE_LEVEL_VULKAN_FUNCTION( vkWaitForFences )
DEVICE_LEVEL_VULKAN_FUNCTION( vkResetFences )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyFence )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroySemaphore )
DEVICE_LEVEL_VULKAN_FUNCTION( vkResetCommandBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkFreeCommandBuffers )
DEVICE_LEVEL_VULKAN_FUNCTION( vkResetCommandPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyCommandPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateBufferView )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyBufferView )
DEVICE_LEVEL_VULKAN_FUNCTION( vkQueueWaitIdle )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateSampler )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateDescriptorSetLayout )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateDescriptorPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkAllocateDescriptorSets )
DEVICE_LEVEL_VULKAN_FUNCTION( vkUpdateDescriptorSets )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdBindDescriptorSets )
DEVICE_LEVEL_VULKAN_FUNCTION( vkFreeDescriptorSets )
DEVICE_LEVEL_VULKAN_FUNCTION( vkResetDescriptorPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyDescriptorPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyDescriptorSetLayout )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroySampler )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateRenderPass )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateFramebuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyFramebuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyRenderPass )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdBeginRenderPass )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdNextSubpass )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdEndRenderPass )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreatePipelineCache )
DEVICE_LEVEL_VULKAN_FUNCTION( vkGetPipelineCacheData )
DEVICE_LEVEL_VULKAN_FUNCTION( vkMergePipelineCaches )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyPipelineCache )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateGraphicsPipelines )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateComputePipelines )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyPipeline )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyEvent )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyQueryPool )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateShaderModule )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyShaderModule )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreatePipelineLayout )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroyPipelineLayout )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdBindPipeline )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdSetViewport )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdSetScissor )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdBindVertexBuffers )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdDraw )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdDrawIndexed )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdDispatch )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdCopyImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdPushConstants )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdClearColorImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdClearDepthStencilImage )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdBindIndexBuffer )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdSetLineWidth )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdSetDepthBias )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdSetBlendConstants )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdExecuteCommands )
DEVICE_LEVEL_VULKAN_FUNCTION( vkCmdClearAttachments )

// extension functions
DEVICE_LEVEL_VULKAN_FUNCTION( vkCreateSwapchainKHR )
DEVICE_LEVEL_VULKAN_FUNCTION( vkGetSwapchainImagesKHR )
DEVICE_LEVEL_VULKAN_FUNCTION( vkAcquireNextImageKHR )
DEVICE_LEVEL_VULKAN_FUNCTION( vkQueuePresentKHR )
DEVICE_LEVEL_VULKAN_FUNCTION( vkDestroySwapchainKHR )

// apparently i'm missing this device memory one?
// DEVICE_LEVEL_VULKAN_FUNCTION( vkGetPhysicalDeviceMemoryProperties )

#undef DEVICE_LEVEL_VULKAN_FUNCTION

// // device level functions from extension
// #ifndef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
// #define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function )
// #endif

// // extension functions
// DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( vkCreateSwapchainKHR )
// DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( vkGetSwapchainImagesKHR )
// DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( vkAcquireNextImageKHR )
// DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( vkQueuePresentKHR )
// DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( vkDestroySwapchainKHR )

// #undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION