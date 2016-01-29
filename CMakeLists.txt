cmake_minimum_required(VERSION 3.4)


project(blah)
include_directories(inc)
set( CMAKE_CXX_FLAGS -std=c++14)

set(SRCS
        test.cpp
        src/slist.cpp inc/slist.hpp inc/types.hpp inc/atomics.hpp inc/fixedtreearray.hpp inc/alloctree.hpp inc/slaballocator.hpp src/slaballocator.cpp inc/syscall.hpp inc/sysconfig.hpp inc/bitops.hpp)


add_executable(blah ${SRCS})
set_target_properties(blah PROPERTIES
    CMAKE_CXX_COMPILER "/usr/bin/clang++"
    LINK_FLAGS "-g -march=haswell -mx32  -nostdlib -fno-builtin -nostartfiles -nodefaultlibs -z max-page-size=0x1000"
    COMPILE_FLAGS "-ggdb -std=c++14 -mx32 -Wall -msse4.2 -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti -nostdlib -nostdinc -mcmodel=small")

