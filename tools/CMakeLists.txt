project(ccl-tools)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(CCL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CCL_EDITORCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/../.editorconfig")
set(CCL_TOPLEVEL "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(src)