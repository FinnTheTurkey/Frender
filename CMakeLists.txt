cmake_minimum_required(VERSION 3.0.0)
project(Frender VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

add_library(Frender Src/Window.cc Src/Renderer.cc)
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
# add_subdirectory(ThirdParty/glm)
target_include_directories(Frender PUBLIC ThirdParty/glm)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
