cmake_minimum_required(VERSION "3.15") # this seems to work for now... 

# set C++ version to 20 i guess
set(CMAKE_CXX_STANDARD 17)

project(PEngine)

# configure CMake to use a "bin" folder for output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")


# needs vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)

# SHADER TARGET:
find_program(GLSL_VALIDATOR glslangValidator HINTS $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/p_render/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/p_render/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/p_render/shaders/*.comp"
    # add stuff for tessellation shaders etc as needed
    )


## iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
    message(STATUS "BUILDING SHADER: ")
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    set(SPIRV "${CMAKE_CURRENT_SOURCE_DIR}/bin/shaders/${FILE_NAME}.spv")
    message(STATUS ${GLSL})
    ##execute glslang command to compile that specific shader
    add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

# add pengine source files
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")
