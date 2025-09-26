##################################################################
#
# Bundle Protocol Library (BPLib) CMake build recipe
#
# This is for the CT module
#
##################################################################

add_library(bplib_ct OBJECT
    src/bplib_ct.c
    src/bplib_ct_ccs.c
    src/bplib_ct_db.c
)

target_include_directories(bplib_ct PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:bplib,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_em,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_eid,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_mem,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_crc,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_pdb,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_rbt,INTERFACE_INCLUDE_DIRECTORIES>
)

# Add unit tests
if(BPLIB_ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif(BPLIB_ENABLE_UNIT_TESTS)
