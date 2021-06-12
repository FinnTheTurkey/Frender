cmake_minimum_required(VERSION 3.0.0)
project(Frender VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(Frender Src/Frender.cpp)
target_include_directories(Frender PUBLIC Include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
