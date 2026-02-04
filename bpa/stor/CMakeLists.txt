##################################################################
#
# Bundle Protocol Library (BPLib) CMake build recipe
#
# This is for the STOR module
#
##################################################################

# We dont want to compile the storage in bplib, that has to be provided by the using application.
# However the general buildsystem breaks if the library is not an OBJECT, so I add some c file that
# provides a throwaway function which does nothing and should not get linked.
add_library(bplib_stor OBJECT
    src/bplib_stor_cmake.c
)

target_include_directories(bplib_stor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    $<TARGET_PROPERTY:bplib,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_em,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_qm,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:bplib_as,INTERFACE_INCLUDE_DIRECTORIES>
)

# Add unit tests
if(BPLIB_ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif(BPLIB_ENABLE_UNIT_TESTS)
