# CMakeLists file for pengine source

# TODO - maybe restructure all of these into subdirectories? might be worth it idk
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

    ## ENGINE COMPONENTS
    "components/Component.cpp"
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
    
    "core/p_render/backend/Context.cpp"
    "core/p_render/backend/frame_context/FrameContext.cpp"
    "core/p_render/backend/wsi/WindowSystem.cpp"
    "core/p_render/backend/shaders/Program.cpp"
    "core/p_render/backend/shaders/ShaderModule.cpp"

    "core/p_render/backend/gui/VulkanGUIHandler.cpp"
    "core/p_render/backend/gui/VulkanGUIData.cpp"
    "core/p_render/backend/PhysicalPass.cpp"

    "core/p_render/VulkanMemoryAllocator.cpp"
    )

# try adding the pengine executable here?
add_executable(pgame WIN32 ${pengine_src}) # forgot to add this win32 flag

# add pengine headers
    # not sure whether to use include_directories() or target_include_directories()
target_include_directories(pgame PUBLIC BEFORE "$../include") # this seems to work!

# try and get vulkan integrated?????
    # apparently CMake makes this easy... 8===D~~~~~~~~~~~~~~
target_include_directories(pgame PUBLIC BEFORE "${Vulkan_INCLUDE_DIRS}")
target_link_libraries(pgame Vulkan::Vulkan)


# not sure what else goes in this one... 