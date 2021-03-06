# CMakeLists file for pengine source

# TODO - look into CMake best practices, not sure if this is the ideal way of organizing the project
set(pengine_src 
    ## ENTRY POINT
    "main.cpp"
    ## 

    ## spirv reflect
    "spirv_reflect/spirv_reflect.c"
    ##

    ## P_ENGINE CORE
    "core/PEngineCore.cpp"

    ### Modes
    "core/engine_mode/PEngineMode.cpp"
    "core/engine_mode/CoreMenuMode.cpp"
    ###

    ## THREAD POOL
    "core/thread_pool/ThreadPool.cpp"
    "core/thread_pool/Thread.cpp"
    "core/thread_pool/job_queue/JobQueue.cpp"
    ## 

    ## LIBRARY INTEGRATION
    "vulkan/vulkan_functions/VulkanFunctions.cpp"
    ##

    ## DearIMGUI SOURCE
    "imgui/imgui.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_tables.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imgui_impl_vulkan.cpp"
    "imgui/imgui_impl_win32.cpp"
    ##

    ## ENGINE TOOLS
        ## still need to think about how this is gonna work...
    "core/engine_tools/EngineTool.cpp"
    ##

    ### P_RENDER
    "core/p_render/PRender.cpp"
    
    ## RENDER GRAPH
    "core/p_render/render_graph/RenderGraph.cpp"
    "core/p_render/render_graph/Shader.cpp"
    "core/p_render/render_graph/pass/Pass.cpp"
    "core/p_render/render_graph/pass/Subpass.cpp"

    "core/p_render/render_graph/resources/RenderResource.cpp"
    "core/p_render/render_graph/resources/ImageResource.cpp"
    "core/p_render/render_graph/resources/BufferResource.cpp"

    ## SCENE
    "core/p_render/scene/Scene.cpp"
    # "core/p_render/scene/Renderable.cpp"
    "core/p_render/scene/mesh/Mesh.cpp"
    "core/p_render/scene/material/Material.cpp"
    
    "core/p_render/backend/Context.cpp"
    "core/p_render/backend/frame_context/FrameContext.cpp"
    "core/p_render/backend/wsi/WindowSystem.cpp"
    "core/p_render/backend/shaders/Program.cpp"
    "core/p_render/backend/shaders/ShaderModule.cpp"

    "core/p_render/backend/gui/VulkanGUIHandler.cpp"
    "core/p_render/backend/gui/VulkanGUIData.cpp"
    "core/p_render/backend/PhysicalPass.cpp"

    ## UTILS
    "core/utils/UniqueID.cpp"

    ## VULKAN MEMORY ALLOCATOR
    "core/p_render/VulkanMemoryAllocator.cpp"

    ## MESHOPT
        # for now i'll include all the algorithms and prune as needed
    "mesh_optimizer/allocator.cpp"
    "mesh_optimizer/clusterizer.cpp"
    "mesh_optimizer/indexcodec.cpp"
    "mesh_optimizer/indexgenerator.cpp"
    "mesh_optimizer/overdrawanalyzer.cpp"
    "mesh_optimizer/overdrawoptimizer.cpp"
    "mesh_optimizer/simplifier.cpp"
    "mesh_optimizer/spatialorder.cpp"
    "mesh_optimizer/stripifier.cpp"
    "mesh_optimizer/vcacheanalyzer.cpp"
    "mesh_optimizer/vcacheoptimizer.cpp"
    "mesh_optimizer/vertexcodec.cpp"
    "mesh_optimizer/vertexfilter.cpp"
    "mesh_optimizer/vfetchanalyzer.cpp"
    "mesh_optimizer/vfetchoptimizer.cpp"
    )

# try adding the pengine executable here?
add_executable(pgame WIN32 ${pengine_src})

# add pengine headers
target_include_directories(pgame PUBLIC BEFORE "$../include") # this seems to work!

# integrate vulkan :)
target_include_directories(pgame PUBLIC BEFORE "${Vulkan_INCLUDE_DIRS}")
target_link_libraries(pgame Vulkan::Vulkan)
