cmake_minimum_required(VERSION 3.0.0)
project(Frender VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

# Include shader parser
include(CMake/ShaderParser.cmake)

####################################################################################################
# This function converts any file into C/C++ source code.
# Example:
# - input file: data.dat
# - output file: data.h
# - variable name declared in output file: DATA
# - data length: sizeof(DATA)
# embed_resource("data.glsl" "data.h" "DATA")
####################################################################################################

function(embed_shader resource_file_name source_file_name variable_name)
    get_filename_component(og_path "Shaders/${resource_file_name}" ABSOLUTE)
    get_filename_component(newg_path "Shaders/build/${resource_file_name}" ABSOLUTE)

    # Compile shaders
    parse_includes_file("${og_path}" "${newg_path}")

    # Check for errors
    glsl_validate("${newg_path}")

    # Write to C source file
    file(READ "${newg_path}" text_content)

    string(REPLACE "\"" "\\\"" content "${text_content}")
    string(REPLACE "\n" "\\n\\\n" content "${content}")

    set(array_definition "static const char ${variable_name}[] = \"${content}\";")

    set(source "// Auto generated file.\n${array_definition}\n")

    file(WRITE "Include/Frender/Shaders/${source_file_name}" "${source}")

endfunction()

embed_shader("stage1.vert" "Stage1Vert.h" "BulkStage1VertSrc")
embed_shader("stage1.frag" "Stage1Frag.h" "BulkStage1FragSrc")

embed_shader("stage2light.vert" "Stage2Vert.h" "Stage2VertSrc")
embed_shader("stage2light.frag" "Stage2Frag.h" "Stage2FragSrc")
embed_shader("stage2directionallight.frag" "Stage2FragD.h" "Stage2FragDSrc")
embed_shader("stage2ambient.frag" "Stage2FragA.h" "Stage2FragASrc")

embed_shader("stage3.vert" "Stage3Vert.h" "Stage3VertSrc")
embed_shader("stage3.frag" "Stage3Frag.h" "Stage3FragSrc")
embed_shader("stage3fxaa.frag" "Stage3FxaaFrag.h" "Stage3FxaaFragSrc")
embed_shader("stage3bloom.frag" "BloomFrag.h" "BloomFragSrc")

embed_shader("unlit.vert" "UnlitVert.h" "UnlitVertSrc")
embed_shader("unlit.frag" "UnlitFrag.h" "UnlitFragSrc")

embed_shader("lit.vert" "LitVert.h" "LitVertSrc")
embed_shader("lit.frag" "LitFrag.h" "LitFragSrc")

embed_shader("equiToCubemap.vert" "EquiToCubemapVert.h" "EquiToCubemapVertSrc")
embed_shader("equiToCubemap.frag" "EquiToCubemapFrag.h" "EquiToCubemapFragSrc")
embed_shader("equiToCubemap_convolute.frag" "EquiToCubemapFrag_convolute.h" "EquiToCubemap_convoluteFragSrc")
embed_shader("equiToCubemap_prefilter.frag" "EquiToCubemapFrag_prefilter.h" "EquiToCubemap_prefilterFragSrc")
embed_shader("IntegrateBRDF.frag" "IntegrateBRDFFrag.h" "IntegrateBRDFFragSrc")
embed_shader("skybox.frag" "SkyboxFrag.h" "SkyboxFragSrc")
embed_shader("skybox.vert" "SkyboxVert.h" "SkyboxVertSrc")
# Disable the defered render
# add_compile_definitions(FRENDER_NO_DEFERED)

add_library(Frender 
    Src/Window.cc
    Src/Renderer.cc
    Src/GLTools.cc
    Src/BulkRenderer.cc

    Src/FrenderTools/FrenderTools.cc
    )
target_include_directories(Frender PUBLIC Include)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# For Wayland
set(GLFW_USE_WAYLAND OFF CACHE BOOL "" FORCE)

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