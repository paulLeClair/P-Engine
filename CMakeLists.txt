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
        # LIBS

        ## BS thread pool (TODO -> FetchContent this)
        src/lib/bs_thread_pool/BS_thread_pool.hpp

        ## VMA (TODO -> evaluate whether this is already imported through vulkan; or if we can FetchContent if not)
        # TODO -> rip this out but in a separate commit so that we can ensure nothing breaks (VMA comes with vulkan sdk now)
        src/lib/vk_mem_alloc/vk_mem_alloc.cpp

        ## SPIRV Reflect (TODO -> get this with fetch content
        src/lib/spirv_reflect/spirv_reflect.h
        src/lib/spirv_reflect/spirv_reflect.cpp

        ## SPIRV header (TODO -> FetchContent)
        "src/lib/spirv/unified1/spirv.h"

        ## DearImGui (TODO -> get this with FetchContent)
        src/lib/dear_imgui/imconfig.h
        src/lib/dear_imgui/imgui.h
        src/lib/dear_imgui/imgui_impl_vulkan.h
        src/lib/dear_imgui/imgui_internal.h
        src/lib/dear_imgui/imstb_rectpack.h
        src/lib/dear_imgui/imstb_textedit.h
        src/lib/dear_imgui/imstb_truetype.h
        src/lib/dear_imgui/imgui.cpp
        src/lib/dear_imgui/imgui_impl_vulkan.cpp
        src/lib/dear_imgui/imgui_tables.cpp
        src/lib/dear_imgui/imgui_widgets.cpp
        src/lib/dear_imgui/imgui_draw.cpp
        src/lib/dear_imgui/imgui_demo.cpp

        src/GraphicsEngine/Backend/Renderer/VulkanGuiThread/VulkanGuiThread.hpp

        # murmur3 hash algorithm (TODO -> potentially fetchcontent, although this one is smaller and not actively updated afaik)
        src/lib/murmur3/MurmurHash3.hpp
        src/lib/murmur3/MurmurHash3.cpp

        # ENGINE CORE

        # THREAD POOL - TODO - implement this in a future issue

        # WINDOW SYSTEM
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSInterface.hpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterface.cpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterface.hpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/OSWindow.h
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/Win32Window/Win32Window.cpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/Win32Window/Win32Window.hpp


        # GRAPHICS ENGINE
        src/GraphicsEngine/GraphicsEngine.hpp

        ## BACKEND
        src/GraphicsEngine/Backend/Backend.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBackend.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBackend.hpp
        src/GraphicsEngine/Backend/Renderer/Renderer.hpp
        src/GraphicsEngine/Backend/Renderer/VulkanRenderer/VulkanRenderer.hpp
        src/GraphicsEngine/Backend/ApplicationContext/ApplicationContext.hpp
        src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.cpp
        src/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanApplicationContext.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstance.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstance.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDevice.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBuffer/VulkanBuffer.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBuffer/VulkanBuffer.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanImage/VulkanImage.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanImage/VulkanImage.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModule.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModule.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanRenderPass/VulkanRenderPass.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPipeline/VulkanPipeline.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.hpp

        src/GraphicsEngine/Scene/Scene.hpp
        src/GraphicsEngine/Scene/ShaderModule/ShaderModule.hpp

        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/MultisampleResolveOperation/MultisampleResolveOperation.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentState/AttachmentState.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentLoadOperation/AttachmentLoadOperation.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/AttachmentStoreOperation/AttachmentStoreOperation.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/ClearValue/ClearValue.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ImageAttachment/ImageAttachment.hpp

        ## RESOURCES
        src/GraphicsEngine/Scene/SceneResources/SceneResource.hpp

        # RENDER GRAPH
        src/GraphicsEngine/Scene/RenderGraph/RenderGraph.hpp

        # RENDER PASS

        # MODELS
        src/GraphicsEngine/Scene/SceneResources/Model/Model.hpp

        # MATERIALS
        src/GraphicsEngine/Scene/SceneResources/Material/Material.hpp

        src/EngineCore/EngineCore.hpp
        src/Application/EngineMode/EngineMode.hpp
        src/Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineMode.cpp
        src/Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineMode.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/Buffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Image/Image.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp
        src/utilities/UniqueIdentifier/UniqueIdentifier.cpp
        src/utilities/UniqueIdentifier/UniqueIdentifier.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.hpp
        src/GraphicsEngine/Scene/Camera/Camera.hpp
        src/GraphicsEngine/Scene/SceneResources/Texture/Texture.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocator.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocator.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPushConstant/VulkanPushConstant.cpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanPushConstant/VulkanPushConstant.hpp
        src/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstant.cpp
        src/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstant.hpp

        # TODO - rip out these bake process things

        src/utilities/ByteArray/ByteArray.cpp
        src/utilities/ByteArray/ByteArray.hpp
        src/GraphicsEngine/Scene/SceneResources/Texture/Sampler/SamplerSettings.hpp
        src/GraphicsEngine/Scene/SceneResources/formats/ResourceFormat/ResourceFormat.hpp

        # GIR
        src/GraphicsEngine/GraphicsIR/GraphicsIntermediateRepresentation.hpp
        src/GraphicsEngine/GraphicsIR/ResourceIR/ImageIR/ImageIR.hpp
        src/GraphicsEngine/GraphicsIR/ResourceIR/BufferIR/BufferIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/GraphicsPipelineIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/ColorBlendStateIR/ColorBlendStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/DepthStencilStateIR/DepthStencilStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/DynamicStateIR/DynamicStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/MultisampleStateIR/MultisampleStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/PrimitiveAssemblyIR/PrimitiveAssemblyIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/RasterizationStateIR/RasterizationStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/TessellationStateIR/TessellationStateIR.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/VertexInputStateIR/VertexInputStateIR.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIR.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/StaticRenderPassIR/StaticRenderPassIR.hpp
        src/GraphicsEngine/GraphicsIR/DrawAttachmentIR/DrawAttachmentIR.hpp
        src/GraphicsEngine/GraphicsIR/ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIR.hpp

        src/GraphicsEngine/Scene/SwapchainRenderTarget/SwapchainRenderTarget.hpp
        src/GraphicsEngine/GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIR.hpp
        src/GraphicsEngine/GraphicsIR/ShaderModuleIR/ShaderModuleIR.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/TextureAttachment/TextureAttachment.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ShaderAttachment/ShaderAttachment.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/DrawAttachment/DrawAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/RenderPassIR.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/GraphicsPipelineConfiguration.hpp
        src/GraphicsEngine/GraphicsIR/RenderGraphIR/RenderGraphIR.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/BufferAttachment/BufferAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/ImageAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/MultisampleResolveOperation/MultisampleResolveOperation.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentState/AttachmentState.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentLoadOperation/AttachmentLoadOperation.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/AttachmentStoreOperation/AttachmentStoreOperation.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/ClearValue/ClearValue.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ImageAttachment/ImageAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/TextureAttachment/TextureAttachment.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/ShaderConstantAttachment/ShaderConstantAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ResourceAttachments/ShaderConstantAttachment/ShaderConstantAttachment.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/ShaderAttachment/ShaderAttachment.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/ColorBlendConfiguration/ColorBlendConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/DepthStencilConfiguration/DepthStencilConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/DynamicStateConfiguration/DynamicStateConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/MultiblendConfiguration/MultisampleConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/PrimitiveAssemblyConfiguration/PrimitiveAssemblyConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/RasterizationConfiguration/RasterizationConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/TessellationConfiguration/TessellationConfiguration.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/GraphicsPipelineConfiguration/VertexInputConfiguration/VertexInputConfiguration.hpp
        src/GraphicsEngine/GraphicsIR/ResourceIR/ResourceIR.hpp
        src/Application/Application.hpp
        src/GraphicsEngine/GraphicsIR/GirGenerator.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanProgram/VulkanProgram.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanProgram/ShaderModuleReflectionData/ShaderModuleReflectionData.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanProgram/VulkanFormatSize.hpp
        src/utilities/ObjectHashPool/ObjectHashPool.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/ResourceDescriptorBindings.hpp

        src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorPool/VulkanDescriptorPool.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSet/VulkanDescriptorSet.hpp
        src/utilities/IntrusiveList/IntrusiveList.hpp
        src/utilities/IntrusiveHashMap/IntrusiveHashMap.hpp
        src/utilities/Hash/Hash.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSet/DescriptorBinding.hpp
        src/utilities/Hash/HashedObject.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanResourceRepository/VulkanResourceRepository.hpp
        src/GraphicsEngine/GraphicsIR/VertexAttributeIR/VertexAttributeIR.hpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanSwapchain/VulkanSwapchain.hpp
        src/GraphicsEngine/Backend/Renderer/PresentationEngine/PresentationEngine.hpp
        src/GraphicsEngine/Backend/Renderer/PresentationEngine/VulkanPresentationEngine/VulkanPresentationEngine.hpp
        src/GraphicsEngine/GraphicsIR/RenderPassIR/DearImguiRenderPassIR/DearImguiRenderPassIR.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/DynamicRenderPass.hpp
        src/GraphicsEngine/Backend/Renderer/VulkanRenderer/VulkanRenderer.cpp
        src/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/XLibWindow/XLibWindow.hpp
        src/GraphicsEngine/Scene/SceneSpace/SceneSpace.hpp
        src/GraphicsEngine/Scene/SceneSpace/Position/Position.hpp
        src/GraphicsEngine/Scene/SceneSpace/Velocity/Velocity.hpp
        src/GraphicsEngine/GraphicsIR/CameraGIR/CameraGIR.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/ModelSegment/ModelSegment.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/Animation/Skeleton/Skeleton.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/Animation/Skeleton/Bone/Bone.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/Animation/Animation.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/Animation/Keyframe/Keyframe.hpp
        src/GraphicsEngine/Scene/SceneResources/Model/Animation/Skeleton/BoneGroup/SkeletonNode.hpp
        src/GraphicsEngine/GraphicsIR/model/ModelIR/ModelIR.hpp
        src/GraphicsEngine/GraphicsIR/model/AnimationIR/AnimationIR.hpp
        src/GraphicsEngine/GraphicsIR/model/KeyframeIR/KeyframeIR.hpp
        src/Application/EngineMode/AnimatedModelDemoMode/AnimatedModelDemoMode.hpp
        src/GraphicsEngine/Scene/SceneSpace/Orientation/Orientation.hpp
        src/GraphicsEngine/Backend/VulkanBackend/Model/VulkanModel.hpp
        src/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/DynamicRenderPassPipelineStateIR/DynamicRenderPassPipelineStateIR.hpp
        src/GraphicsEngine/GraphicsIR/ResourceIR/FormatIR/FormatIR.hpp
        src/GraphicsEngine/Scene/SceneResources/GeometryBinding/GeometryBinding.hpp
        src/GraphicsEngine/Scene/SceneResources/GeometryBinding/IndexData/IndexDataBinding.hpp
        src/GraphicsEngine/Scene/SceneResources/GeometryBinding/VertexData/VertexDataBinding.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderPass/RenderPassCore.hpp
        src/GraphicsEngine/GraphicsIR/VertexInputBindingIR/VertexInputBindingIR.hpp
        src/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocationHandle.hpp
        src/utilities/ArenaAllocator/ArenaAllocator.hpp
        src/GraphicsEngine/Scene/Light/PointLight.hpp
        src/GraphicsEngine/GraphicsIR/light/PointLightIR/PointLightIR.hpp
        src/GraphicsEngine/Scene/RenderGraph/RenderGraph.cpp
        src/GraphicsEngine/Scene/Scene.cpp
        src/utilities/GuiWidget/GuiWidget.hpp
        src/GraphicsEngine/Backend/Renderer/VulkanGuiThread/VulkanGuiThread.cpp
        src/GraphicsEngine/Backend/Renderer/VulkanRenderer/Frame.hpp
        src/utilities/assimp/SceneImporter.cpp
        src/utilities/assimp/SceneImporter.hpp
        src/utilities/assimp/SkeletalAnimation.cpp
        src/utilities/assimp/SkeletalAnimation.hpp
        src/utilities/assimp/AssimpData.hpp
        src/utilities/GuiWidget/assimp/AssimpAnimationEditor.hpp
        src/utilities/GuiWidget/Table.hpp)

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
#
#message(STATUS "GTest found: ${GTest_FOUND}")


set(PEngine_test
        test/GraphicsEngine/Backend/FrameExecutionController/PFrameExecutionController/PFrameExecutionControllerTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/VulkanApplicationContext/VulkanApplicationContextTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanBackendTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanInstance/VulkanInstanceTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanLogicalDevice/VulkanLogicalDeviceTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanPhysicalDevice/VulkanPhysicalDeviceTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanShaderModule/VulkanShaderModuleTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanRenderPass/VulkanRenderPassTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanImage/VulkanImageTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanGeometry/VulkanIndexedMesh/VulkanIndexedMeshTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanBuffer/VulkanBufferTest.cpp
        test/Application/ApplicationTest.cpp test/Application/utilities/IntrusiveList/IntrusiveListTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Model/ModelTest.cpp

        test/GraphicsEngine/Scene/ShaderModule/ShaderModule/ShaderModuleTest.cpp

        #        test/Application/PEngineCore/PEngineTest.cpp
        #        test/Application/ThreadPool/PThreadPool/PThreadPoolTest.cpp
        #        test/Application/ThreadPool/PThreadPool/Thread/ThreadTest.cpp
        #        test/Application/ThreadPool/PThreadPool/JobQueue/JobQueueTest.cpp
        #        test/Application/ThreadPool/PThreadPool/Job/JobTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/OSInterface/VulkanOSInterface/VulkanOSInterfaceTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/OSInterface/OSWindow/Win32Window/Win32WindowTest.cpp
        test/Application/EngineMode/CoreMenuEngineMode/CoreMenuEngineModeTest.cpp
        test/EngineCore/EngineCoreTest.cpp
        test/Application/utilities/UniqueIdentifier/UniqueIdentifierTest.cpp
        test/Application/utilities/RawDataContainer/RawDataContainerTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Material/MaterialTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Image/ImageTest.cpp
        test/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstantTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Texture/TextureTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/IndexBufferTest.cpp
        #        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexBufferTest.cpp
        test/GraphicsEngine/Scene/RenderGraph/RenderGraphTest.cpp
        test/GraphicsEngine/Scene/RenderGraph/RenderPass/RenderPassTest.cpp
        test/GraphicsEngine/Scene/RenderGraph/SwapchainRenderTarget/SwapchainRenderTargetTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/StorageTexelBuffer/StorageTexelBufferTest.cpp
        test/GraphicsEngine/Scene/SceneView/SceneViewTest.cpp
        test/GraphicsEngine/GraphicsIR/GraphicsIntermediateRepresentationTest.cpp
        test/GraphicsEngine/GraphicsIR/ShaderModuleIR/ShaderModuleIRTest.cpp
        test/GraphicsEngine/GraphicsIR/ResourceIR/ImageIR/ImageIRTest.cpp
        test/GraphicsEngine/GraphicsIR/ResourceIR/BufferIR/BufferIRTest.cpp
        test/GraphicsEngine/GraphicsIR/ResourceIR/ShaderConstantIR/ShaderConstantIRTest.cpp
        test/GraphicsEngine/GraphicsIR/RenderPassIR/RenderPassIRTest.cpp
        test/GraphicsEngine/GraphicsIR/RenderGraphIR/RenderGraphIRTest.cpp
        test/GraphicsEngine/GraphicsIR/GraphicsPipelineIR/GraphicsPipelineIRTest.cpp
        test/GraphicsEngine/GraphicsIR/DrawCommandsIR/DrawCommandsIRTest.cpp
        test/GraphicsEngine/GraphicsIR/RenderPassIR/DynamicRenderPassIR/DynamicRenderPassIRTest.cpp
        test/GraphicsEngine/GraphicsIR/RenderPassIR/StaticRenderPassIR/StaticRenderPassIRTest.cpp
        test/GraphicsEngine/GraphicsIR/ShaderModuleIR/SpirVShaderModuleIR/SpirVShaderModuleIRTest.cpp
        test/Application/EngineMode/EngineModeTest.cpp
        test/Application/utilities/ObjectHashPool/ObjectHashPoolTest.cpp
        test/Application/utilities/IntrusiveHashMap/IntrusiveHashMapTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocatorTest.cpp
        test/GraphicsEngine/Backend/VulkanBackend/VulkanBufferSuballocator/VulkanBufferSuballocatorTest.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utilities)
add_executable(pgameTest WIN32 ${PEngine_src} ${PEngine_test})
target_include_directories(pgameTest PUBLIC BEFORE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(pgameTest Vulkan::Vulkan gtest_main assimp ${BULLET_LIBRARIES})
add_test(NAME AllTests COMMAND pgameTest)

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
FetchContent_Declare(Bullet
        GIT_REPOSITORY https://github.com/bulletphysics/bullet3
        GIT_TAG master
        OVERRIDE_FIND_PACKAGE)
set(BUILD_UNIT_TESTS OFF)

# TODO - verify that this Bullet integration actually works

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS} build/_deps/bullet-src/src)
set(BUILD_UNIT_TESTS OFF)
target_link_libraries(pgame ${BULLET_LIBRARIES})
