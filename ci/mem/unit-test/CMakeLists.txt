##################################################################
#
# Coverage test build recipe
#
# This CMake file contains the recipe for building the coverage tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

# Create stubs (for external use)
add_library(bplib_mem_stubs STATIC
    stubs/bplib_mem_stubs.c
)

target_include_directories(bplib_mem_stubs PUBLIC
    $<TARGET_PROPERTY:bplib_mem,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(bplib_mem_stubs PUBLIC ut_assert)


# Create unit test object
if (BPLIB_MEM_IMPL STREQUAL CFE)
    add_library(utobj_bplib_mem OBJECT
        ../src/bplib_mem.c
        ../src/bplib_cfe_allocator.c
    )
elseif (BPLIB_MEM_IMPL STREQUAL BEN)
    add_library(utobj_bplib_mem OBJECT
        ../src/bplib_mem.c
        ../src/bplib_ben_allocator.c
    )
else()
    add_library(utobj_bplib_mem OBJECT
        ../src/bplib_mem.c
        ../src/bplib_std_allocator.c
    )
endif()

target_compile_definitions(utobj_bplib_mem PRIVATE
    $<TARGET_PROPERTY:bplib_mem,COMPILE_DEFINITIONS>
    $<TARGET_PROPERTY:ut_coverage_compile,INTERFACE_COMPILE_DEFINITIONS>
)

target_compile_options(utobj_bplib_mem PRIVATE
    $<TARGET_PROPERTY:bplib_mem,COMPILE_OPTIONS>
    $<TARGET_PROPERTY:ut_coverage_compile,INTERFACE_COMPILE_OPTIONS>
)

target_include_directories(utobj_bplib_mem PRIVATE
    $<TARGET_PROPERTY:bplib_mem,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:ut_coverage_compile,INTERFACE_INCLUDE_DIRECTORIES>
)

# Create test runner executable
add_executable(coverage-bplib_mem-testrunner
    utilities/bplib_mem_test_utils.c
    bplib_mem_test.c
    $<TARGET_OBJECTS:utobj_bplib_mem>
)

target_include_directories(coverage-bplib_mem-testrunner PRIVATE
    ../src
    utilities/
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:bplib_mem,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_em,INTERFACE_INCLUDE_DIRECTORIES>
)

if (BPLIB_MEM_IMPL STREQUAL CFE)
    target_link_libraries(coverage-bplib_mem-testrunner PUBLIC
        ut_coverage_link
        ut_assert
        ut_core_api_stubs
        bplib_em_stubs
    )
else()
    target_link_libraries(coverage-bplib_mem-testrunner PUBLIC
        ut_coverage_link
        ut_assert
        bplib_em_stubs
    )
endif()

add_test(coverage-bplib_mem-testrunner coverage-bplib_mem-testrunner)

# Install the executables to a staging area for test in cross environments
if (INSTALL_TARGET_LIST)
    foreach(TGT ${INSTALL_TARGET_LIST})
        install(TARGETS coverage-bplib_mem-testrunner DESTINATION ${TGT}/${UT_INSTALL_SUBDIR})
    endforeach()
endif()
