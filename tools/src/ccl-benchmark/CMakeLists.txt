file(GLOB_RECURSE BENCHMARK_SOURCES *.cpp *.hpp *.c *.h)

add_executable(ccl-benchmark ${BENCHMARK_SOURCES} ${CCL_SOURCES} ${CCL_EDITORCONFIG})
target_include_directories(ccl-benchmark PRIVATE "${CCL_INCLUDE_DIR}")

# Check compiler flags
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-pedantic" CCL_PEDANTIC_ENABLED)

if(CCL_PEDANTIC_ENABLED)
    target_compile_options(ccl-benchmark PRIVATE "-pedantic")
endif()

check_cxx_compiler_flag("-O2" CCL_O2_ENABLED)

if(CCL_O2_ENABLED)
    target_compile_options(ccl-benchmark PRIVATE "-O2")
endif()

# Visual Studio specific stuff
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${CCL_INCLUDE_DIR} FILES ${CCL_SOURCES})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "ccl-benchmark" FILES ${BENCHMARK_SOURCES})
    set_property(DIRECTORY ${CCL_TOPLEVEL} PROPERTY VS_STARTUP_PROJECT ccl-benchmark)
endif()