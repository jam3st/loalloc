cmake_minimum_required(VERSION 3.4)


project(blah)
include_directories(inc)
set( CMAKE_CXX_FLAGS "-std=c++14 -mx32 -ggdb -march=haswell -std=c++14 -m64 -Wall -msse4.2 -fsized-deallocation -fno-stack-protector -fno-exceptions -fno-rtti -nostdlib -nostdinc -mcmodel=small")

set(SRCS
        test.cpp
        inc/slist.hpp inc/types.hpp inc/atomics.hpp inc/slaballocator.hpp inc/syscall.hpp inc/sysconfig.hpp inc/bitops.hpp inc/spinlock.hpp inc/debug.hpp slaballocator.cpp)


set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -m64 -O3")

add_executable(blah ${SRCS})
set_target_properties(blah PROPERTIES
    LINK_FLAGS "-g -m64 -march=haswell -nostdlib  -nostartfiles -nodefaultlibs -z max-page-size=0x1000"
    )

