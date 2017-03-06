cmake_minimum_required(VERSION 3.7)


project(blah)
include_directories(inc)
set( CMAKE_CXX_FLAGS "-mx32 -ggdb -march=haswell -std=c++14  -Wall -msse4.2 -fsized-deallocation -fno-stack-protector -fno-exceptions -mno-red-zone -fno-rtti -nostdlib -mcmodel=small -fno-common")

set(SRCS
        test.cpp
        inc/types inc/atomics inc/regionallocator inc/syscall inc/sysconfig.hpp inc/bitops inc/spinlock inc/debug slaballocator.cpp inc/radtree)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(blah ${SRCS})
set_target_properties(blah PROPERTIES
    LINK_FLAGS "-ggdb -mx32  -fsized-deallocation  -march=haswell -nostdlib -z max-page-size=0x1000"
)

