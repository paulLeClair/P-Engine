cmake_minimum_required(VERSION "3.20")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./build")
set(CMAKE_CXX_STANDARD 17)

project(PEngine)

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
    ##execute glslang command to compile that specific shader
    add_custom_command(
            OUTPUT ${SPIRV_BINARY}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV_BINARY}
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV_BINARY})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

enable_testing()


set(Pgame_main
        src/main.cpp)

set(PEngine_src
        # LIBS
        ## VMA
        src/lib/vk_mem_alloc/vk_mem_alloc.cpp

        ## SPIRV Reflect
        src/lib/spirv_reflect/spirv_reflect.h
        src/lib/spirv_reflect/spirv_reflect.c

        ## DearImGui
        src/lib/dear_imgui/imconfig.h
        src/lib/dear_imgui/imgui.h
        src/lib/dear_imgui/imgui_impl_vulkan.h
        src/lib/dear_imgui/imgui_impl_win32.h
        src/lib/dear_imgui/imgui_internal.h
        src/lib/dear_imgui/imstb_rectpack.h
        src/lib/dear_imgui/imstb_textedit.h
        src/lib/dear_imgui/imstb_truetype.h
        src/lib/dear_imgui/imgui.cpp
        src/lib/dear_imgui/imgui_impl_vulkan.cpp
        src/lib/dear_imgui/imgui_impl_win32.cpp
        src/lib/dear_imgui/imgui_tables.cpp
        src/lib/dear_imgui/imgui_widgets.cpp
        src/lib/dear_imgui/imgui_draw.cpp

        # ENGINE CORE
        src/EngineCore/ThreadPool/ThreadPool.hpp
        src/EngineCore/ThreadPool/PThreadPool/PThreadPool.cpp
        src/EngineCore/ThreadPool/PThreadPool/PThreadPool.hpp
        src/EngineCore/ThreadPool/PThreadPool/Thread/Thread.cpp
        src/EngineCore/ThreadPool/PThreadPool/Thread/Thread.hpp
        src/EngineCore/ThreadPool/PThreadPool/Job/Job.cpp
        src/EngineCore/ThreadPool/PThreadPool/Job/Job.hpp
        src/EngineCore/ThreadPool/PThreadPool/JobQueue/JobQueue.cpp
        src/EngineCore/ThreadPool/PThreadPool/JobQueue/JobQueue.hpp

        # WINDOW SYSTEM
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/WindowSystem.hpp
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.cpp
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/PWindowSystem/PWindowSystem.hpp
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/OSWindow/OSWindow.h
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/OSWindow/Win32Window/Win32Window.cpp
        src/GraphicsEngine/Backend/ApplicationContext/WindowSystem/OSWindow/Win32Window/Win32Window.hpp


        # GRAPHICS ENGINE
        src/GraphicsEngine/GraphicsEngine.hpp

        ## BACKEND
        src/GraphicsEngine/Backend/Backend.hpp
        src/GraphicsEngine/Backend/PBackend/PBackend.cpp
        src/GraphicsEngine/Backend/PBackend/PBackend.hpp
        src/GraphicsEngine/Backend/FrameExecutionController/FrameExecutionController.hpp
        src/GraphicsEngine/Backend/FrameExecutionController/PFrameExecutionController/PFrameExecutionController.cpp
        src/GraphicsEngine/Backend/FrameExecutionController/PFrameExecutionController/PFrameExecutionController.hpp
        src/GraphicsEngine/Backend/ApplicationContext/ApplicationContext.hpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/PApplicationContext.cpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/PApplicationContext.hpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstance.cpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstance.hpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanLogicalDevice/VulkanLogicalDevice.cpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanLogicalDevice/VulkanLogicalDevice.hpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.cpp
        src/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDevice.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBuffer.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBuffer.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanImage/VulkanImage.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanImage/VulkanImage.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderPass/VulkanDynamicRenderPass/VulkanDynamicRenderPass.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanShaderModule/VulkanShaderModule.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanShaderModule/VulkanShaderModule.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderable/VulkanIndexedMesh/VulkanIndexedMesh.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderable/VulkanIndexedMesh/VulkanIndexedMesh.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderPass/VulkanRenderPass.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderable/VulkanRenderable.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanPipeline/VulkanPipeline.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanPipeline/VulkanComputePipeline/VulkanComputePipeline.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanPipeline/VulkanGraphicsPipeline/VulkanGraphicsPipeline.hpp

        ## SCENE MANAGER
        src/GraphicsEngine/Scene/Scene.hpp
        src/GraphicsEngine/Scene/PScene/PScene.cpp
        src/GraphicsEngine/Scene/PScene/PScene.hpp
        src/GraphicsEngine/Scene/ShaderModule/ShaderModule.hpp
        src/GraphicsEngine/Scene/ShaderModule/PShaderModule/PShaderModule.cpp
        src/GraphicsEngine/Scene/ShaderModule/PShaderModule/PShaderModule.hpp

        ## RESOURCES / RESOURCE MANAGER
        src/GraphicsEngine/Scene/SceneResources/SceneResource.hpp

        # RENDER GRAPH
        src/GraphicsEngine/Scene/SceneRenderGraph/SceneRenderGraph.hpp
        src/GraphicsEngine/Scene/SceneRenderGraph/PSceneRenderGraph/PSceneRenderGraph.cpp
        src/GraphicsEngine/Scene/SceneRenderGraph/PSceneRenderGraph/PSceneRenderGraph.hpp

        # RENDER PASS
        src/GraphicsEngine/Scene/SceneRenderGraph/RenderPass/RenderPass.hpp
        src/GraphicsEngine/Scene/SceneRenderGraph/RenderPass/PRenderPass/PRenderPass.cpp
        src/GraphicsEngine/Scene/SceneRenderGraph/RenderPass/PRenderPass/PRenderPass.hpp

        # MODELS
        src/GraphicsEngine/Scene/SceneResources/Model/Model.hpp

        # RENDERABLES
        src/GraphicsEngine/Scene/SceneResources/Renderable/Renderable.hpp

        # MATERIALS
        src/GraphicsEngine/Scene/SceneResources/Material/Material.hpp

        src/EngineCore/PEngineCore/PEngineCore.cpp
        src/EngineCore/PEngineCore/PEngineCore.hpp
        src/GraphicsEngine/PGraphicsEngine/PGraphicsEngine.cpp
        src/GraphicsEngine/PGraphicsEngine/PGraphicsEngine.hpp
        src/EngineCore/EngineMode/EngineMode.hpp src/EngineCore/EngineMode/PEngineMode/PEngineMode.cpp
        src/EngineCore/EngineMode/PEngineMode/PEngineMode.hpp
        src/EngineCore/EngineMode/PEngineMode/CoreMenuEngineMode/CoreMenuEngineMode.cpp
        src/EngineCore/EngineMode/PEngineMode/CoreMenuEngineMode/CoreMenuEngineMode.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/Buffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Image/Image.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderPass/DearImguiVulkanRenderPass/DearImguiVulkanRenderPass.hpp
        src/GraphicsEngine/Scene/SceneRenderGraph/ImguiRenderPass/ImguiRenderPass.hpp
        src/GraphicsEngine/Scene/SceneRenderGraph/ImguiRenderPass/PImguiRenderPass/PImguiRenderPass.cpp
        src/GraphicsEngine/Scene/SceneRenderGraph/ImguiRenderPass/PImguiRenderPass/PImguiRenderPass.hpp
        src/GraphicsEngine/Backend/FrameExecutionController/Frame/Frame.hpp
        src/GraphicsEngine/Backend/FrameExecutionController/Frame/PFrame/PFrame.cpp
        src/GraphicsEngine/Backend/FrameExecutionController/Frame/PFrame/PFrame.hpp
        src/GraphicsEngine/Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.cpp
        src/GraphicsEngine/Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMesh.hpp
        src/EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.cpp
        src/EngineCore/utilities/UniqueIdentifier/UniqueIdentifier.hpp
        src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.cpp
        src/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMesh.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderable/VulkanVertexList/VulkanVertexList.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanRenderable/VulkanVertexList/VulkanVertexList.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocation/VulkanBufferSuballocation.hpp
        src/GraphicsEngine/Scene/SceneView/SceneView.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanCameraView/VulkanCameraView.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanCameraView/VulkanCameraView.hpp
        src/GraphicsEngine/Scene/SceneView/PSceneView/PSceneView.cpp
        src/GraphicsEngine/Scene/SceneView/PSceneView/PSceneView.hpp
        src/GraphicsEngine/Scene/SceneResources/Texture/Texture.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocator.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBufferSuballocator/VulkanBufferSuballocator.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanDescriptorSetAllocator/VulkanDescriptorSetAllocator.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanPushConstant/VulkanPushConstant.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanPushConstant/VulkanPushConstant.hpp
        src/GraphicsEngine/Backend/PBackend/VulkanTexture/VulkanTexture.cpp
        src/GraphicsEngine/Backend/PBackend/VulkanTexture/VulkanTexture.hpp
        src/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstant.cpp
        src/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstant.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/RenderPassBakeProcess/RenderPassBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/RenderPassBakeProcess/RenderPassBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/BufferBakeProcess/BufferBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/BufferBakeProcess/BufferBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ImageBakeProcess/ImageBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ImageBakeProcess/ImageBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/MaterialBakeProcess/MaterialBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/MaterialBakeProcess/MaterialBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ModelBakeProcess/ModelBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ModelBakeProcess/ModelBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/renderable_bake_processes/IndexedTriangleMeshBakeProcess/IndexedTriangleMeshBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/renderable_bake_processes/IndexedTriangleMeshBakeProcess/IndexedTriangleMeshBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/renderable_bake_processes/TriangleMeshBakeProcess/TriangleMeshBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/renderable_bake_processes/TriangleMeshBakeProcess/TriangleMeshBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ShaderConstantBakeProcess/ShaderConstantBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/ShaderConstantBakeProcess/ShaderConstantBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/TextureBakeProcess/TextureBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/resource_bake_processes/TextureBakeProcess/TextureBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/SceneViewBakeProcess/SceneViewBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/SceneViewBakeProcess/SceneViewBakeProcess.hpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/ShaderModuleBakeProcess/ShaderModuleBakeProcess.cpp
        src/GraphicsEngine/Backend/PBackend/scene_bake_processes/ShaderModuleBakeProcess/ShaderModuleBakeProcess.hpp

        src/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBuffer.hpp
        src/EngineCore/utilities/RawDataContainer/RawDataContainer.cpp
        src/EngineCore/utilities/RawDataContainer/RawDataContainer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/LongIndexBuffer/LongIndexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/LongIndexBuffer/LongIndexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/IntIndexBuffer/IntIndexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/IntIndexBuffer/IntIndexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/CharIndexBuffer/CharIndexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/CharIndexBuffer/CharIndexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/UntemplatedIndexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/UntemplatedIndexBuffer/UntemplatedIndexBuffer.hpp

        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexTypes/VertexTypes.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionOnlyVertexBuffer/PositionOnlyVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionOnlyVertexBuffer/PositionOnlyVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorVertexBuffer/PositionColorVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorVertexBuffer/PositionColorVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalVertexBuffer/PositionColorNormalVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalVertexBuffer/PositionColorNormalVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVVertexBuffer/PositionColorNormalUVVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVVertexBuffer/PositionColorNormalUVVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentVertexBuffer/PositionColorNormalUVTangentVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentVertexBuffer/PositionColorNormalUVTangentVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentBitangentVertexBuffer/PositionColorNormalUVTangentBitangentVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentBitangentVertexBuffer/PositionColorNormalUVTangentBitangentVertexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/UntemplatedVertexBuffer.cpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/UntemplatedVertexBuffer.hpp

        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/IndexBuffer.hpp
        src/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/IndexTypes/IndexTypes.hpp
        src/GraphicsEngine/Scene/SceneResources/Sampler/Sampler.hpp
        src/GraphicsEngine/Scene/SceneResources/TexelFormat/TexelFormat.hpp
        src/GraphicsEngine/Scene/SceneResources/TexelFormat/TexelTypes.hpp src/GraphicsEngine/Scene/SceneResources/TexelFormat/R8G8B8A8_SRGB_TexelFormat/R8G8B8A8_SRGB_TexelFormat.hpp)

# SET UP PGAME EXECUTABLE
add_executable(pgame WIN32 ${Pgame_main} ${PEngine_src})
target_include_directories(pgame PUBLIC BEFORE ${Vulkan_INCLUDE_DIRS} src/lib/vk_mem_alloc/vk_mem_alloc.h)
target_link_libraries(pgame Vulkan::Vulkan)

# SETUP GOOGLETEST
include(FetchContent)
FetchContent_Declare(
        googletest
        # UPDATE COMMIT HASH REGULARLY (the part before the ".zip")
        URL https://github.com/google/googletest/archive/f2d71f0be2ff2230789cfaf37d4c4f38db5b1c7c.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


set(PTest_main
        test/gtestMain.cpp)

set(PEngine_test
        ${PTest_main}

        # GRAPHICS

        ## BACKEND MANAGER
        test/GraphicsEngine/Backend/FrameExecutionController/PFrameExecutionController/PFrameExecutionControllerTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/PApplicationContextTest.cpp
        test/GraphicsEngine/Backend/PBackend/PBackendTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/VulkanInstanceTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanLogicalDevice/VulkanLogicalDeviceTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanPhysicalDevice/VulkanPhysicalDeviceTest.cpp
        test/GraphicsEngine/Backend/PBackend/VulkanShaderModule/VulkanShaderModuleTest.cpp
        test/GraphicsEngine/Backend/PBackend/VulkanRenderPass/VulkanRenderPassTest.cpp
        test/GraphicsEngine/Backend/PBackend/VulkanImage/VulkanImageTest.cpp
        test/GraphicsEngine/Backend/PBackend/VulkanGeometry/VulkanIndexedMesh/VulkanIndexedMeshTest.cpp
        test/GraphicsEngine/Backend/PBackend/VulkanBuffer/VulkanBufferTest.cpp

        ## SCENE MANAGER
        test/GraphicsEngine/Scene/PScene/PSceneTest.cpp

        ## RENDER GRAPH
        test/GraphicsEngine/Scene/RenderGraph/RenderPass/PRenderPass/PRenderPassTest.cpp
        test/GraphicsEngine/Scene/RenderGraph/PRenderGraph/PRenderGraphTest.cpp

        ## MODEL
        test/GraphicsEngine/Scene/SceneResources/Model/ModelTest.cpp

        ## RENDERABLE

        ## SHADER MODULE
        test/GraphicsEngine/Scene/ShaderModule/PShaderModule/PShaderModuleTest.cpp

        test/EngineCore/PEngineCore/PEngineTest.cpp test/EngineCore/ThreadPool/PThreadPool/PThreadPoolTest.cpp
        test/EngineCore/ThreadPool/PThreadPool/Thread/ThreadTest.cpp
        test/EngineCore/ThreadPool/PThreadPool/JobQueue/JobQueueTest.cpp
        test/EngineCore/ThreadPool/PThreadPool/Job/JobTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/WindowSystem/PWindowSystem/PWindowSystemTest.cpp
        test/GraphicsEngine/Backend/ApplicationContext/WindowSystem/OSWindow/Win32Window/Win32WindowTest.cpp
        test/GraphicsEngine/PGraphicsEngineTest/PGraphicsEngineTest.cpp
        test/EngineCore/EngineMode/PEngineMode/PCoreMenuEngineMode/CoreMenuEngineModeTest.cpp

        test/EngineCore/utilities/UniqueIdentifier/UniqueIdentifierTest.cpp
        test/EngineCore/utilities/RawDataContainer/RawDataContainerTest.cpp
        test/EngineCore/EngineMode/PEngineMode/PEngineModeTest.cpp
        test/GraphicsEngine/Backend/FrameExecutionController/Frame/PFrame/PFrameTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Material/MaterialTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Renderable/TriangleMesh/TriangleMeshTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Renderable/IndexedTriangleMesh/IndexedTriangleMeshTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Image/ImageTest.cpp
        test/GraphicsEngine/Scene/SceneResources/ShaderConstant/ShaderConstantTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Texture/TextureTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/StorageBuffer/StorageBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/TexelBuffer/TexelBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/UniformBuffer/UniformBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorVertexBuffer/PositionColorVertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalVertexBuffer/PositionColorNormalVertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentVertexBuffer/PositionColorNormalUVTangentVertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVTangentBitangentVertexBuffer/PositionColorNormalUVTangentBitangentVertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionOnlyVertexBuffer/PositionOnlyVertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/IndexBuffer/IndexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/VertexBufferTest.cpp
        test/GraphicsEngine/Scene/SceneResources/Buffer/VertexBuffer/UntemplatedVertexBuffer/PositionColorNormalUVVertexBuffer/PositionColorNormalUVVertexBufferTest.cpp test/GraphicsEngine/Scene/SceneResources/Buffer/Sampler/SamplerTest.cpp)

add_executable(pgameTest WIN32 ${PEngine_src} ${PEngine_test})
target_include_directories(pgameTest PUBLIC BEFORE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(pgameTest Vulkan::Vulkan gtest)

#include(GoogleTest)
#gtest_discover_tests(pgameTest)
#add_test(NAME test/GraphicsEngine/Backend/ApplicationContext/PApplicationContext/VulkanInstance/TestVulkanInstanceManager.cpp COMMAND pgameTest)
#gtest_add_tests(pgameTest ${PEngine_test})