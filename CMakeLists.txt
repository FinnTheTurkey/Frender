cmake_minimum_required(VERSION 3.0.0)
project(Frender VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

####################################################################################################
# This function converts any file into C/C++ source code.
# Example:
# - input file: data.dat
# - output file: data.h
# - variable name declared in output file: DATA
# - data length: sizeof(DATA)
# embed_resource("data.glsl" "data.h" "DATA")
####################################################################################################

function(embed_resource resource_file_name source_file_name variable_name)

    file(READ ${resource_file_name} text_content)

    string(REPLACE "\n" "\\n\\\n" content "${text_content}")

    set(array_definition "static const char ${variable_name}[] = \"${content}\";")

    set(source "// Auto generated file.\n${array_definition}\n")

    file(WRITE "${source_file_name}" "${source}")

endfunction()

embed_resource("Shaders/stage1.vert" "Include/Frender/Shaders/Stage1Vert.h" "BulkStage1VertSrc")
embed_resource("Shaders/stage1.frag" "Include/Frender/Shaders/Stage1Frag.h" "BulkStage1FragSrc")

add_library(Frender 
    Src/Window.cc
    Src/Renderer.cc
    Src/GLTools.cc
    Src/BulkRenderer.cc
    )
target_include_directories(Frender PUBLIC Include)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/glfw)
target_link_libraries(Frender glfw)

# Glad
add_subdirectory(ThirdParty/glad)
target_link_libraries(Frender glad)

# GLM
# Make sure GLM always creates matricies with values that actually work
add_compile_definitions(GLM_FORCE_CTOR_INIT)
target_include_directories(Frender PUBLIC ThirdParty/glm)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
