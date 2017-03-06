#include "debug"
#include "atomics"
#include "syscall"
#include "regionallocator"

/*
extern "C"
void *memset(void *ptr, Gx::int32_t len, Gx::size_t val) {
    Gx::size_t *p = static_cast<Gx::size_t*>(ptr);
    Gx::ssize_t num = Gx::alignToBits(len, Gx::alignmentBits) / sizeof(Gx::size_t);
    while (num-- > 0) {
        *p++ = val;
    }
    return (ptr);
}
*/
namespace Gx {
    template<size_t size>
    struct syscallBaseType;
    template<>
    struct syscallBaseType<4> {
        constexpr static size_t value = 0x40000000u;
    };
    template<>
    struct syscallBaseType<8> {
        constexpr static size_t value = 0x0u;
    };
    constexpr size_t syscallBase = syscallBaseType<sizeof(size_t)>::value;

    size_t strlen(char const *src) {
        auto ret = 0;
        while (*src++ != '\0') {
            ret++;
        }
        return ret;
    }

    struct CpuIdInfo {
        Gx::uint32_t eax;
        Gx::uint32_t ebx;
        Gx::uint32_t ecx;
        Gx::uint32_t edx;
    };

    void exit(size_t ret) {
        constexpr unsigned sysExit = syscallBase + 60;
        __asm__ __volatile__("syscall;" : : "a"(sysExit), "D"(ret) : "rcx", "r11", "memory");
    }

    CpuIdInfo cpuId(Gx::uint32_t func, Gx::uint32_t subFunc) {
        CpuIdInfo result;
        __asm__ __volatile__ ("cpuid;" : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx) : "a"(func), "c"(subFunc) : );
        return result;

    }

    uint32_t getNumCpus() {
        return cpuId(0xBu, 0x1u).ebx;
    }

    uint32_t getCpu() {
        uint32_t result = 0u;
        constexpr unsigned sysGetCpu = syscallBase + 309;
        __asm__ __volatile__("syscall;" : : "a"(sysGetCpu), "D"(&result) : "rcx");
        return result;

    }

    uint32_t clone(void (*func)(void *const, size_t const), void *allocator, size_t id) {
        constexpr size_t sysClone = syscallBase + 56;
        constexpr size_t cloneVM = 0x00000100u;
        constexpr size_t cloneThread = 0x00010000u;
        constexpr size_t cloneParent = 0x00000800u;
        constexpr size_t flags = cloneVM | cloneParent | cloneThread;
        auto stack = static_cast<uint64_t *>(mmap(nullptr, minPageFrameSize)) + minPageFrameSize / sizeof(uint64_t) - 3;
        stack[0] = id;
        stack[1] = reinterpret_cast<uintptr_t>(allocator);
        stack[2] = reinterpret_cast<uintptr_t>(func);
        int32_t res;
        __asm__ __volatile__("syscall;"
                "andl %%eax, %%eax;"
                "jnz 0f;"
                "pop %%rsi;"
                "pop %%rdi;"
                "ret;"
                "0:;" : "=a"(res) : "a"(sysClone), "S"(stack), "D"(flags) : "cc", "rcx", "r11" );
        return res;
    }

    uint32_t getCore() {
        unsigned long core;
        __asm__ __volatile__ ("rdtscp;": "=c" (core));
        return core;
    }

    inline uint64_t GetCounter() {
        union {
            Gx::uint64_t as64;
            struct {
                Gx::uint32_t low;
                Gx::uint32_t high;
            };
        } ret{.as64 = 0ull};
        __asm__ __volatile__ ("rdtscp;": "=a" (ret.low), "=d" (ret.high) : : );
        return ret.as64;
    }

    void *initBrk() {
        constexpr size_t sysBrk = syscallBase + 12;
        void *newBrk = nullptr;
        __asm__ __volatile__("syscall;" : "=a"(newBrk): "a"(sysBrk), "D"(0u) :  "rcx", "r11");
        return newBrk;
    }

    size_t extendBrk(void *const alloc, size_t const len) {
        constexpr size_t sysBrk = syscallBase + 12;
        uintptr_t allocVal = reinterpret_cast<uintptr_t>(alloc);
        uintptr_t requestedEnd = allocVal + len;
        if (allocVal + len != requestedEnd) { __builtin_trap(); }
        uintptr_t newEnd = 0u;
        __asm__ __volatile__("syscall" : "=a"(newEnd) : "a"(sysBrk), "D"(requestedEnd) : "rcx", "r11");
        return newEnd - allocVal;
    }

    void debug(char const *const str) {
        if (Debug::enabled) {
            constexpr unsigned sysWrite = syscallBase + 1;
            constexpr int stderr = 2;
            const size_t len = strlen(str);
            __asm__ __volatile__("syscall;" : : "a"(sysWrite), "D"(stderr), "S"(str), "d"(len) : "rcx", "r11");
        }
    }
}


using namespace Gx;


void *operator new[](size_t const howMuch) {
    void* ret = Bitmaps::allocator->allocate(howMuch);
    return ret;
}


void *operator new(size_t const howMuch) {
    return Bitmaps::allocator->allocate(howMuch);
}

void operator delete[](void *const what) noexcept {
//    const size_t howmuch = 100;
//    Bitmaps::allocator->deAllocate(what, howmuch);
}

void operator delete(void *const what, size_t const howMuch) noexcept {
    Bitmaps::allocator->deAllocate(what, howMuch);
}

void operator delete(void *const what) noexcept {
    size_t howMuch = 0u;
    Bitmaps::allocator->deAllocate(what, howMuch);
}


uint32_t xorshift128(uint32_t &x, uint32_t &y, uint32_t &z, uint32_t &w) {
    uint32_t t = x;
    t ^= t << 11;
    t ^= t >> 8;
    x = y; y = z; z = w;
    w ^= w >> 19;
    w ^= t;
    if(w > 1000000u) {
        return w >> 16;
    }
    if(w == 0u) {
        return  1u;
    }
    return w;
}

extern "C"
void _start() {
    Bitmaps::init();
    Bitmaps::allocator->dump(true);
    uint32_t x = 1;
    uint32_t y = 2;
    uint32_t z = 3;
    uint32_t w = 4;
    struct blah {
        void* somePointer;
    };

    Thash<void*, 30u - defaultAlignmentBits> thash;

    Debug::start() + thash.lenBits() + " " + thash.firstLenInBits + " " + thash.maxLevel + Debug::end;
    thash.set(reinterpret_cast<void*>(0x1234), 0x13);
    thash.get(0u);
    exit(0);
    constexpr size_t max = 256 * 1024;
    struct Allocs {
        void* what;
        size_t howMuch;
    };
    static_assert(sizeof(void*) == 4, "");
    auto test = reinterpret_cast<Allocs*>(operator new(max * sizeof(Allocs)));
    for(auto i = 0u; i < max; ++i) {
        test[i].howMuch = (xorshift128(x,y,z,w) >> 16) + 1u;
        test[i].what = operator new(test[i].howMuch);
    }

    for(auto i = 0u; i < max; ++i) {
        if((xorshift128(x,y,z,w) & 0x100u) != 0u) {
            if(test[i].what != nullptr) {
                Bitmaps::allocator->deAllocate(test[i].what, test[i].howMuch);
                test[i].what = nullptr;
            }
        }
        if((i % 100000) == 0) {  Bitmaps::allocator->dump(true); }
    }
    for(auto stress = 0; stress < 16; ++stress) {
        for (auto i = 0u; i < max; ++i) {
            if ((xorshift128(x, y, z, w) & 0x101u) != 0u) {
                if (test[i].what != nullptr) {
                    Bitmaps::allocator->deAllocate(test[i].what, test[i].howMuch);
                    test[i].what = nullptr;
                }
            }
            if((i % 100000) == 0) {  Bitmaps::allocator->dump(true); }
        }

        for (auto i = 0u; i < max; ++i) {
            if ((xorshift128(x, y, z, w) & 0x101u) != 0u) {
                if (test[i].what == nullptr) {
                    test[i].howMuch = xorshift128(x, y, z, w);
                    test[i].what = operator new(test[i].howMuch);
                }
            }
            if((i % 100000) == 0) {  Bitmaps::allocator->dump(true); }
        }
        Debug::start() + stress + Debug::end;
    }
    Bitmaps::allocator->dump(true);
    for(auto i = 0u; i < max; ++i) {
        if(test[i].what != nullptr) {
            Bitmaps::allocator->deAllocate(test[i].what, test[i].howMuch);
            test[i].what = nullptr;
            Bitmaps::allocator->dump(false);
        }
    }
    Bitmaps::allocator->deAllocate(test, max * sizeof(Allocs));
    Bitmaps::allocator->dump(true);
    exit(0);
}
