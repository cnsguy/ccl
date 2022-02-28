file(GLOB_RECURSE TEST_SOURCES *.c *.h)

add_executable(ccl-test ${TEST_SOURCES} ${CCL_SOURCES} ${CCL_EDITORCONFIG})
target_include_directories(ccl-test PRIVATE "${CCL_INCLUDE_DIR}")

# Check compiler flags
include(CheckCCompilerFlag)
check_c_compiler_flag("-pedantic" CCL_PEDANTIC_ENABLED)

if(CCL_PEDANTIC_ENABLED)
    target_compile_options(ccl-test PRIVATE "-pedantic")
endif()

set(__SAVED_CRL ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES "-fsanitize=address;asan")
check_c_compiler_flag("-fsanitize=address" CCL_ASAN_ENABLED)
set(CMAKE_REQUIRED_LIBRARIES ${__SAVED_CRL})

if(CCL_ASAN_ENABLED)
    target_compile_options(ccl-test PRIVATE "-fsanitize=address")
    target_link_libraries(ccl-test asan)
endif()

# Visual Studio specific stuff
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${CCL_INCLUDE_DIR} FILES ${CCL_SOURCES})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "ccl-test" FILES ${TEST_SOURCES})
    set_property(DIRECTORY ${CCL_TOPLEVEL} PROPERTY VS_STARTUP_PROJECT ccl-test)
endif()