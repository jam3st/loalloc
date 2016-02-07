#include "debug.hpp"
#include "types.hpp"
#include "slist.hpp"
#include "atomics.hpp"
#include "syscall.hpp"
#include "slaballocator.hpp"

extern "C"
void *memset(void *ptr, int len, Gx::size_t num) {
    unsigned char *p = (unsigned char *) ptr;
    while (len > 0) {
        *p = num;
        p++;
        len--;
    }
    return (ptr);
}

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
        uint32_t result;
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

void *operator new(size_t howMuch) {
    return (void *) (0x01323u);
}

void operator delete(void *const what, size_t const howmuch) noexcept {

}

struct Allocs {
    void *addr;
    size_t const len;
} allocs0[]{
        {
                nullptr, 16
        },
        {
                nullptr, 32
        },
        {
                nullptr, 48
        },
        {
                nullptr, 64
        },
        {
                nullptr, 3094
        },
        {
                nullptr, 16
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 3094
        },
        {
                nullptr, 16
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 1024
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 25u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 119u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 95u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 38u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 88u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 96u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 37u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 33u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 112u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 43u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 137u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 80u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 55u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 99u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 50u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 75u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 142u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 86u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 128u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 16u
        },
        {
                nullptr, 134u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 87u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 54u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 49u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 30u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 67u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 60u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 100u
        },
        {
                nullptr, 113u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 72u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 108u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 111u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 36u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 19u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 127u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 56u
        },
        {
                nullptr, 47u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 5u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 97u
        },
        {
                nullptr, 63u
        },
        {
                nullptr, 121u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 13u
        },
        {
                nullptr, 124u
        },
        {
                nullptr, 21u
        },
        {
                nullptr, 123u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 57u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 77u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 29u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 71u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 85u
        },
        {
                nullptr, 76u
        },
        {
                nullptr, 92u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 52u
        },
        {
                nullptr, 105u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 129u
        },
        {
                nullptr, 48u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 22u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 4u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 53u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 103u
        },
        {
                nullptr, 106u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 115u
        },
        {
                nullptr, 135u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 126u
        },
        {
                nullptr, 35u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 125u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 14u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 81u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 84u
        },
        {
                nullptr, 120u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 70u
        },
        {
                nullptr, 26u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 6u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 140u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 130u
        },
        {
                nullptr, 24u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 102u
        },
        {
                nullptr, 7u
        },
        {
                nullptr, 91u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 107u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 83u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 78u
        },
        {
                nullptr, 139u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 20u
        },
        {
                nullptr, 118u
        },
        {
                nullptr, 133u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 46u
        },
        {
                nullptr, 18u
        },
        {
                nullptr, 1u
        },
        {
                nullptr, 51u
        },
        {
                nullptr, 28u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 23u
        },
        {
                nullptr, 27u
        },
        {
                nullptr, 39u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 62u
        },
        {
                nullptr, 61u
        },
        {
                nullptr, 116u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 34u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 59u
        },
        {
                nullptr, 10u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 117u
        },
        {
                nullptr, 79u
        },
        {
                nullptr, 89u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 122u
        },
        {
                nullptr, 109u
        },
        {
                nullptr, 90u
        },
        {
                nullptr, 114u
        },
        {
                nullptr, 65u
        },
        {
                nullptr, 136u
        },
        {
                nullptr, 131u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 45u
        },
        {
                nullptr, 17u
        },
        {
                nullptr, 3u
        },
        {
                nullptr, 68u
        },
        {
                nullptr, 44u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 40u
        },
        {
                nullptr, 58u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 101u
        },
        {
                nullptr, 32u
        },
        {
                nullptr, 31u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 8u
        },
        {
                nullptr, 64u
        },
        {
                nullptr, 82u
        },
        {
                nullptr, 66u
        },
        {
                nullptr, 74u
        },
        {
                nullptr, 94u
        },
        {
                nullptr, 42u
        },
        {
                nullptr, 11u
        },
        {
                nullptr, 41u
        },
        {
                nullptr, 15u
        },
        {
                nullptr, 132u
        },
        {
                nullptr, 9u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 104u
        },
        {
                nullptr, 73u
        },
        {
                nullptr, 98u
        },
        {
                nullptr, 93u
        },
        {
                nullptr, 138u
        },
        {
                nullptr, 2u
        },
        {
                nullptr, 110u
        },
        {
                nullptr, 141u
        },
        {
                nullptr, 69u
        },
        {
                nullptr, 12u
        },
        {
                nullptr, 42u
        }
};

template<size_t len>
void odds(Allocs allocs[], void(*op)(Allocs &)) {
    for (auto i = 1u; i < len / sizeof(allocs[0]); i = i + 2) {
        op(allocs[i]);
        Bitmaps::allocator->dump();
    }
}

template<size_t len>
void evens(Allocs allocs[], void(*op)(Allocs &)) {
    for (auto i = 0u; i < len / sizeof(allocs[0]); i = i + 2) {
        op(allocs[i]);
        Bitmaps::allocator->dump();
    }
}

template<size_t len>
void forward(Allocs allocs[], void(*op)(Allocs &)) {
    for (auto i = 0u; i < len / sizeof(allocs[0]); ++i) {
        op(allocs[i]);
        Bitmaps::allocator->dump();
    }
}

extern "C"
void _start(void) {
    Bitmaps::init();
    Debug::start() + "Start " + Bitmaps::allocator + Debug::end;
    auto allocate = [](decltype(allocs0[0]) &a) {
        if (a.addr != nullptr) {
            __builtin_trap();
        }
        a.addr = Bitmaps::allocator->allocate(a.len);
        memset(a.addr, a.len, 0xa5);
    };
    auto deAllocate = [](decltype(allocs0[0]) &a) {
        if (a.addr == nullptr) {
            __builtin_trap();
        }
        memset(a.addr, a.len, 0xc8);
        Bitmaps::allocator->deAllocate(a.addr, a.len);
        a.addr = nullptr;
    };

    forward<sizeof(allocs0)>(allocs0, allocate);
    odds<sizeof(allocs0)>(allocs0, deAllocate);
    evens<sizeof(allocs0)>(allocs0, deAllocate);
    exit(0);
}
