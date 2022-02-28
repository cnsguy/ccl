file(GLOB_RECURSE CCL_SOURCES "${CCL_INCLUDE_DIR}/ccl/*.h")

option(CCL_BUILD_BENCHMARK "Build ccl-benchmark" ON)
option(CCL_BUILD_TEST "Build ccl-test" ON)

if(${CCL_BUILD_BENCHMARK})
    add_subdirectory(ccl-benchmark)
endif()

if(${CCL_BUILD_TEST})
    add_subdirectory(ccl-test)
endif()