##################################################################
#
# Bundle Protocol Library (BPLib) CMake build recipe
#
# This is for the MEM module
#
##################################################################

if (BPLIB_MEM_IMPL STREQUAL CFE)
    message (STATUS "**************HERE")
    add_library(bplib_mem OBJECT
        src/bplib_mem.c
        src/bplib_cfe_allocator.c
    )
elseif (BPLIB_MEM_IMPL STREQUAL BEN_ALLOC)
    add_library(bplib_mem OBJECT
        src/bplib_mem.c
        src/bplib_ben_allocator.c
    )
else()
    add_library(bplib_mem OBJECT
        src/bplib_mem.c
        src/bplib_std_allocator.c
    )
endif()

target_include_directories(bplib_mem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:bplib,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_em,INTERFACE_INCLUDE_DIRECTORIES>

)

# Add unit tests
if(BPLIB_ENABLE_UNIT_TESTS)
   add_subdirectory(unit-test)
endif(BPLIB_ENABLE_UNIT_TESTS)
