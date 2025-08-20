cmake_minimum_required(VERSION "3.20")

cmake_policy(SET CMP0091 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./build")
set(CMAKE_CXX_STANDARD 20)

project(PEngine CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE INTERNAL "") # I think this makes it so that it uses a dynamic-linked CRT
if (MSVC)
    #    add_compile_options(
    #            "$<$<CONFIG:>:/MD>" #---------|
    #            "$<$<CONFIG:Debug>:/MDd>" #---|-- Dynamically link the runtime libraries
    #            "$<$<CONFIG:Release>:/MD>" #--|
    #    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MDd" CACHE INTERNAL "")
endif ()

find_package(Vulkan 1.3 REQUIRED)

find_program(GLSL_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.comp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.tesc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.tese"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.geom")

## iterate each shader
foreach (GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER: ")
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    set(SPIRV_BINARY "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/bin/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    message(STATUS "spir-v binary output dir: ${SPIRV_BINARY}")
    ##execute glslang command to compile that specific shader
    add_custom_command(
            OUTPUT ${SPIRV_BINARY}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV_BINARY} -g # DEBUG INFORMATION ENABLED WITH -g
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_BINARY})
endforeach (GLSL)

message(STATUS "Spir-V binary files: ${SPIRV_BINARY_FILES}")

add_custom_target(
        Shaders
        ALL
        DEPENDS ${SPIRV_BINARY_FILES}
)


set(Pgame_main
        src/main.cpp
)

set(PEngine_src
        src/Application/Application.hpp
        src/Application/EngineMode/AnimatedModelDemoMode/AnimatedModelDemoMode.hpp
        src/Application/EngineMode/EngineMode.hpp
        src/Application/OSInterface/OSInterface.hpp
        src/Application/OSInterface/OSWindow/OSWindow.h
        src/Application/OSInterface/OSWindow/Win32Window/Win32Window.cpp
        src/Application/OSInterface/OSWindow/Win32Window/Win32Window.hpp
        src/Application/OSInterface/OSWindow/XLibWindow/XLibWindow.hpp
        src/GirEngine/Backend/Backend.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanApplicationContext/VulkanApplicationContext.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanApplicationContext/VulkanApplicationContext.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanBackend.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanBackend.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanBuffer/VulkanBuffer.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanBuffer/VulkanBuffer.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocationHandle.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocator.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocator.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/ResourceDescriptorBindings.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorPool/VulkanDescriptorPool.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSet/DescriptorBinding.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSet/VulkanDescriptorSet.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanGuiThread/VulkanGuiContext.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanGuiThread/VulkanGuiContext.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanImage/VulkanImage.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanImage/VulkanImage.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstance.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstance.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanModel/VulkanModel.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanOSInterface/VulkanOsInterface.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanOSInterface/VulkanOsInterface.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPipeline/VulkanPipeline.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPresentationEngine/VulkanPresentationEngine.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanProgram/ShaderModuleReflectionData/ShaderModuleReflectionData.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanProgram/VulkanFormatSize.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanProgram/VulkanProgram.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanPushConstant/VulkanPushConstant.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanPushConstant/VulkanPushConstant.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderer/Frame.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderer/VulkanRenderer.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderer/VulkanRenderer.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanRenderPass/VulkanRenderPass.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanResourceRepository/VulkanResourceRepository.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModule.cpp
        src/GirEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModule.hpp
        src/GirEngine/Backend/VulkanBackend/VulkanSwapchain/VulkanSwapchain.hpp
        src/GirEngine/Frontend/Frontend.hpp
        src/GirEngine/Frontend/Scene/Camera/Camera.hpp
        src/GirEngine/Frontend/Scene/Light/PointLight.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderGraph.cpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderGraph.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/DrawAttachment/DrawAttachment.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/DynamicRenderPass.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/ColorBlendConfiguration/ColorBlendConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/DepthStencilConfiguration/DepthStencilConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/DynamicStateConfiguration/DynamicStateConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/GraphicsPipelineConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/MultiblendConfiguration/MultisampleConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/PrimitiveAssemblyConfiguration/PrimitiveAssemblyConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/RasterizationConfiguration/RasterizationConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/TessellationConfiguration/TessellationConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/VertexInputConfiguration/VertexInputConfiguration.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentLoadOperation/AttachmentLoadOperation.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentState/AttachmentState.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentStoreOperation/AttachmentStoreOperation.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/ClearValue/ClearValue.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/ImageAttachment.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ImageAttachment/MultisampleResolveOperation/MultisampleResolveOperation.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/RenderPassCore.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ShaderAttachment/ShaderAttachment.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/ShaderConstantAttachment/ShaderConstantAttachment.hpp
        src/GirEngine/Frontend/Scene/RenderGraph/RenderPass/TextureAttachment/TextureAttachment.hpp
        src/GirEngine/Frontend/Scene/Scene.cpp
        src/GirEngine/Frontend/Scene/Scene.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Buffer/Buffer.hpp
        src/GirEngine/Frontend/Scene/SceneResources/formats/ResourceFormat/ResourceFormat.hpp
        src/GirEngine/Frontend/Scene/SceneResources/GeometryBinding/GeometryBinding.hpp
        src/GirEngine/Frontend/Scene/SceneResources/GeometryBinding/IndexData/IndexDataBinding.hpp
        src/GirEngine/Frontend/Scene/SceneResources/GeometryBinding/VertexData/VertexDataBinding.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Image/Image.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Material/Material.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Model/Model.hpp
        src/GirEngine/Frontend/Scene/SceneResources/SceneResource.hpp
        src/GirEngine/Frontend/Scene/SceneResources/ShaderConstant/ShaderConstant.cpp
        src/GirEngine/Frontend/Scene/SceneResources/ShaderConstant/ShaderConstant.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Texture/Sampler/SamplerSettings.hpp
        src/GirEngine/Frontend/Scene/SceneResources/Texture/Texture.hpp
        src/GirEngine/Frontend/Scene/SceneSpace/Orientation/Orientation.hpp
        src/GirEngine/Frontend/Scene/SceneSpace/Position/Position.hpp
        src/GirEngine/Frontend/Scene/SceneSpace/SceneSpace.hpp
        src/GirEngine/Frontend/Scene/SceneSpace/Velocity/Velocity.hpp
        src/GirEngine/Frontend/Scene/ShaderModule/ShaderModule.hpp
        src/GirEngine/Frontend/Scene/SwapchainRenderTarget/SwapchainRenderTarget.hpp
        src/GirEngine/GirEngine.hpp
        src/GirEngine/GraphicsIR/DrawAttachmentIR/DrawAttachmentIR.hpp
        src/GirEngine/GraphicsIR/GraphicsIntermediateRepresentation.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/ColorBlendStateIR/ColorBlendStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/DepthStencilStateIR/DepthStencilStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/DynamicRenderPassPipelineStateIR/DynamicRenderPassPipelineStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/DynamicStateIR/DynamicStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/GraphicsPipelineIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/MultisampleStateIR/MultisampleStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/PrimitiveAssemblyIR/PrimitiveAssemblyIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/RasterizationStateIR/RasterizationStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/TessellationStateIR/TessellationStateIR.hpp
        src/GirEngine/GraphicsIR/GraphicsPipelineIR/VertexInputStateIR/VertexInputStateIR.hpp
        src/GirEngine/GraphicsIR/model/AnimationIR/AnimationIR.hpp
        src/GirEngine/GraphicsIR/model/KeyframeIR/KeyframeIR.hpp
        src/GirEngine/GraphicsIR/model/ModelIR/ModelIR.hpp
        src/GirEngine/GraphicsIR/RenderGraphIR/RenderGraphIR.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/DearImguiRenderPassIR/DearImguiRenderPassIR.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/RenderPassIR.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/BufferAttachment/BufferAttachment.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentLoadOperation/AttachmentLoadOperation.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentState/AttachmentState.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentStoreOperation/AttachmentStoreOperation.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/ClearValue/ClearValue.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/ImageAttachment.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/MultisampleResolveOperation/MultisampleResolveOperation.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ShaderConstantAttachment/ShaderConstantAttachment.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ResourceAttachments/TextureAttachment/TextureAttachment.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/ShaderAttachment/ShaderAttachment.hpp
        src/GirEngine/GraphicsIR/RenderPassIR/StaticRenderPassIR/StaticRenderPassIR.hpp
        src/GirEngine/GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp
        src/GirEngine/GraphicsIR/ResourceIR/FormatIR/FormatIR.hpp
        src/GirEngine/GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp
        src/GirEngine/GraphicsIR/ResourceIR/ResourceIR.hpp
        src/GirEngine/GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp
        src/GirEngine/GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp
        src/GirEngine/GraphicsIR/ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIR.hpp
        src/GirEngine/GraphicsIR/VertexAttributeIR/VertexAttributeIR.hpp
        src/GirEngine/GraphicsIR/VertexInputBindingIR/VertexInputBindingIR.hpp
        src/lib/bs_thread_pool/BS_thread_pool.hpp
        src/lib/dear_imgui/imconfig.h
        src/lib/dear_imgui/imgui.cpp
        src/lib/dear_imgui/imgui.h
        src/lib/dear_imgui/imgui_demo.cpp
        src/lib/dear_imgui/imgui_draw.cpp
        src/lib/dear_imgui/imgui_impl_vulkan.cpp
        src/lib/dear_imgui/imgui_impl_vulkan.h
        src/lib/dear_imgui/imgui_impl_win32.cpp
        src/lib/dear_imgui/imgui_impl_win32.h
        src/lib/dear_imgui/imgui_internal.h
        src/lib/dear_imgui/imgui_tables.cpp
        src/lib/dear_imgui/imgui_widgets.cpp
        src/lib/dear_imgui/imstb_rectpack.h
        src/lib/dear_imgui/imstb_textedit.h
        src/lib/dear_imgui/imstb_truetype.h
        src/lib/murmur3/MurmurHash3.cpp
        src/lib/murmur3/MurmurHash3.hpp
        src/lib/spirv/unified1/spirv.h
        src/lib/spirv_reflect/spirv_reflect.c
        src/lib/spirv_reflect/spirv_reflect.h
        src/lib/vk_mem_alloc/vk_mem_alloc.cpp
        src/lib/vk_mem_alloc/vk_mem_alloc.h
        src/utilities/assimp/AssimpData.hpp
        src/utilities/assimp/SceneImporter.cpp
        src/utilities/assimp/SceneImporter.hpp
        src/utilities/assimp/SkeletalAnimation.cpp
        src/utilities/assimp/SkeletalAnimation.hpp
        src/utilities/ByteArray/ByteArray.cpp
        src/utilities/ByteArray/ByteArray.hpp
        src/utilities/GuiWidget/assimp/AssimpAnimationEditor.hpp
        src/utilities/GuiWidget/GuiWidget.hpp
        src/utilities/GuiWidget/Table.hpp
        src/utilities/Hash/Hash.hpp
        src/utilities/Hash/HashedObject.hpp
        src/utilities/IntrusiveHashMap/IntrusiveHashMap.hpp
        src/utilities/IntrusiveList/IntrusiveList.hpp
        src/utilities/ObjectHashPool/ObjectHashPool.hpp
        src/utilities/UniqueIdentifier/UniqueIdentifier.cpp
        src/utilities/UniqueIdentifier/UniqueIdentifier.hpp
)

if (WIN32)
    list(APPEND PEngine_src
            src/lib/dear_imgui/imgui_impl_win32.cpp
            src/lib/dear_imgui/imgui_impl_win32.h)

endif (WIN32)

if (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif (UNIX)

# SET UP PGAME EXECUTABLE
add_executable(pgame WIN32 ${Pgame_main} ${PEngine_src})
target_include_directories(
        pgame
        PUBLIC BEFORE ${Vulkan_INCLUDE_DIRS}
        src/lib/vk_mem_alloc/vk_mem_alloc.h
)
target_link_libraries(pgame Vulkan::Vulkan)

if (UNIX)
    find_package(X11 REQUIRED)
    if (X11_FOUND)
        message(STATUS "X11 found: ${X11_LIBRARIES}")
        target_include_directories(pgame PUBLIC AFTER "${X11_INCLUDE_DIR}/X11")
        target_link_directories(pgame PUBLIC "${X11_INCLUDE_DIR}/X11")
        target_link_libraries(pgame ${X11_LIBRARIES})
    endif (X11_FOUND)

    #actually it might be way easier to just use glfw
    find_package(glfw3 3.3 REQUIRED)

    target_link_libraries(pgame glfw)

endif (UNIX)

# SETUP GOOGLETEST
#include(FetchContent)
#FetchContent_Declare(
#        googletest
#        # UPDATE COMMIT HASH REGULARLY (the part before the ".zip")
#        DOWNLOAD_EXTRACT_TIMESTAMP ON
#        URL https://github.com/google/googletest/archive/1d17ea141d2c11b8917d2c7d029f1c4e2b9769b2.zip
#)
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(googletest)
#
#enable_testing()
# TODO -> re-enable testing

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utilities)


# SETUP BOOST
# TODO - figure out a more general way to do this; we don't want to hardcode a path for Boost
# even if we just end up sticking in a new (custom) BOOST_PATH argument that's required or something
# (another alternative that might be better: find a way to only include the specific boost libs we're using
# via fetchcontent (not sure how easy that is though))
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/SDKs/boost_1_84_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/SDKs/boost_1_84_0/libs")
set(Boost_NO_WARN_NEW_VERSIONS 1)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.83.0 REQUIRED)
message(STATUS "Boost found: " ${BOOST_FOUND})

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(pgame Boost::dynamic_linking)

# add fetchcontent for assimp
include(FetchContent)
FetchContent_Declare(assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG master
        OVERRIDE_FIND_PACKAGE) # not sure if this is necessary but assimp seems to work
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)
include_directories(pgame PUBLIC build/_deps/assimp-src/include)
include_directories(build/_deps/assimp-src/include)
include_directories(build/_deps/assimp-build/include/assimp)
target_link_libraries(pgame assimp)

# Bullet Physics integration
#FetchContent_Declare(Bullet
#        GIT_REPOSITORY https://github.com/bulletphysics/bullet3
#        GIT_TAG master
#        OVERRIDE_FIND_PACKAGE)
#set(BUILD_UNIT_TESTS OFF)

#find_package(Bullet REQUIRED)
#include_directories(${BULLET_INCLUDE_DIRS} build/_deps/bullet-src/src)
#set(BUILD_UNIT_TESTS OFF)
#target_link_libraries(pgame ${BULLET_LIBRARIES})
