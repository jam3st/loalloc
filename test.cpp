#include "types.hpp"
#include "slist.hpp"
#include "atomics.hpp"
#include "fixedtreearray.hpp"
#include "syscall.hpp"
//#include "slaballocator.hpp"

namespace Gx {
	void dumpString(char const *const str);

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
		constexpr unsigned sysExit = 0x40000000 + 60;
		__asm__ __volatile__("syscall;" : : "a"(sysExit), "D"(ret));
	}

	CpuIdInfo cpuId(Gx::uint32_t func, Gx::uint32_t subFunc) {
		CpuIdInfo result;
		__asm__ __volatile__ ("cpuid;" : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx) : "a"(func), "c"(subFunc));
		return result;

	}

	uint32_t getNumCpus() {
		return cpuId(0xBu, 0x1u).ebx;
	}

	uint32_t getCpu() {
		uint32_t result;
		constexpr unsigned sysGetCpu = 0x40000000 + 309;
		__asm__ __volatile__("syscall;" : : "a"(sysGetCpu), "D"(&result) : "memory");
		return result;

	}



	uint32_t clone (void (*func)(void* const, size_t const), void* allocator, size_t id) {
		constexpr uint32_t sysClone = 0x40000000u + 56;
		constexpr uint32_t cloneVM = 0x00000100u;
		constexpr uint32_t cloneThread = 0x00010000u;
		constexpr uint32_t cloneParent = 0x00000800u;
		constexpr uint32_t flags = cloneVM | cloneParent | cloneThread;
		auto stack = static_cast<uint64_t*>(mmap(nullptr, minPageFrameSize)) + minPageFrameSize / sizeof(uint64_t) - 3;
		stack[0] = id;
		stack[1] = reinterpret_cast<uint32_t>(allocator);
		stack[2] = reinterpret_cast<uint32_t>(func);
		int32_t res;
		__asm__ __volatile__("syscall;"
		                     "andl %%eax, %%eax;"
				 "jnz 0f;"
				 "pop %%rsi;"
				 "pop %%rdi;"
				 "ret;"
				 "0:;" : "=a"(res) : "a"(sysClone), "S"(stack), "D"(flags) : );
		return res;
	}

	uint32_t getCore() {
		unsigned long core;
		__asm__ __volatile__ ("rdtscp;": "=c" (core));
		return core;
	}

	__attribute((always_inline)) inline Gx::uint64_t GetCounter() {
		union {
			Gx::uint64_t as64;
			struct {
				Gx::uint32_t low;
				Gx::uint32_t high;
			};
		} ret;
		__asm__ __volatile__ ("rdtscp;": "=a" (ret.low), "=d" (ret.high));
		return ret.as64;
	}

//	void* initBrk() {
//		constexpr size_t sysBrk = 0x40000000 + 12;
//		SlabAlloctor * newBrk;
//		__asm__ __volatile__("syscall;" : "=a"(newBrk): "a"(sysBrk), "D"(0u));
//		return newBrk;
//	}

	size_t extendBrk(void* const alloc, size_t const len) {
		constexpr size_t sysBrk = 0x40000000 + 12;
		uintptr_t requestedEnd = reinterpret_cast<uintptr_t>(alloc) + len;
		uintptr_t newEnd;
		__asm__ __volatile__("syscall;" : "=a"(newEnd) : "a"(sysBrk), "D"(requestedEnd));
		return newEnd - reinterpret_cast<uintptr_t >(alloc);
	}

//	void*SlabAllocator::allocateFromBss(size_t const len) {
//		modify.lockWriting();
//		auto oldHeapStart = heapSize - lastEndPos;
//		auto allocLen = alignToBits(sizeof(MemItem) + len, defaultAlignmentBits);
//		if(allocLen > lastEndPos) {
//			heapSize = extendBrk(this, heapSize + alignToBits(allocLen, pagesSizesBits[0]));
//		}
//		lastEndPos = 0u;
//		modify.unlockWriting();
//		MemItem& item{*reinterpret_cast<MemItem*>(reinterpret_cast<uintptr_t>(this) + oldHeapStart)};
//		item.len = len;
//		item.next = nullptr;
//		item.prev = nullptr;
//		return nullptr;
//	}





	void print(char const *const str, size_t const len) {
		constexpr unsigned sysWrite = 0x40000000 + 1;
		constexpr int stderr = 2;
		__asm__ __volatile__("syscall;" : : "a"(sysWrite), "D"(stderr), "S"(str), "d"(len));
	}


	uint8_t toPrintHex(Gx::uint64_t const val) {
		if (val < 10) {
			return val + '0';
		} else {
			return val - 10 + 'a';
		}
	}

	uint64_t getBits(Gx::uint64_t const val, Gx::size_t const start, Gx::size_t const end) {
		return (val >> start) & ((1u << (end - start + 1)) - 1);
	}

	void dumpChar(char const value) {
		print(&value, 1);
	}

	void dump64(uint64_t const value) {
		char str[20];
		size_t pos = 0;
		for (ssize_t i = 63; i >= 0; i = i - 4) {
			auto c = getBits(value, i - 3, i);
			if(c != 0 || pos != 0) {
				str[pos++] = toPrintHex(c);
			}
		}
		if(pos == 0) {
			str[pos++] = toPrintHex(0);
		}
		print(str, pos);
	}

	void dumpString(char const *const str) {
		print(str, strlen(str));
	}

//	void doIt(SlabAlloctor * const alloc, size_t const id) {
//		for(int i = 0; i < 100000; ++i) {
//			alloc->modify.lockWriting();
//			alloc->modify.unlockWriting();
//			alloc->modify.lockReading();
//			alloc->modify.unLockRead();
//		}
//		Gx::exit(0);
//	}
// Driver program to test above functions
	bool btreeTest(SlabAllocator<BTreeNode>& allocator) {
		BTree t(allocator);
		for (int i = 0; i < 1000; ++i) {
			t.insert(t, 100);
		}
		t.traverse();
		for(int i= 0; i < 1000; ++i)
			t.insert(t, 100);
		t.traverse();
		t.insert(t, 107);
		t.traverse();
		t.insert(t, 100);
		t.traverse();
		t.insert(t, 111);
		t.traverse();
		t.insert(t, 123);
		t.traverse();
		t.insert(t, 133);
		t.traverse();
		t.insert(t, 123);
		t.traverse();
		t.insert(t, 111);
		t.traverse();
		t.insert(t, 111);
		t.traverse();
		t.insert(t, 113);
		t.traverse();
		t.insert(t, 114);
		t.traverse();
		t.insert(t, 115);
		t.traverse();
		t.insert(t, 118);
		t.traverse();
		t.insert(t, 116);
		t.traverse();
		t.insert(t, 119);
		t.traverse();
		t.insert(t, 124);
		t.traverse();
		t.insert(t, 125);
		t.traverse();
		t.insert(t, 126);
		t.traverse();
		t.insert(t, 121);
		t.traverse();
		t.insert(t, 114);
		t.traverse();
		t.insert(t, 115);
		t.traverse();
		t.insert(t, 120);
		t.traverse();
		t.insert(t, 122);
		t.traverse();
		t.insert(t, 121);
		t.traverse();
		t.insert(t, 117);
		t.traverse();
		t.insert(t, 112);
		t.traverse();
		t.insert(t, 116);
		t.traverse();
		t.remove(104);
		t.traverse();
		t.remove(104);
		t.traverse();
		t.remove(104);
		t.traverse();
		t.remove(104);
		t.traverse();
		t.remove(104);
		t.traverse();
		t.remove(104);
		t.traverse();
		return 0;
	}
}

void* operator new(unsigned int howMuch) {
	return 0;
}
void operator delete(void* what, unsigned int howmuch) noexcept {
	void* tmp = what;
}

using namespace Gx;
extern "C"
void _start(void) {

	auto x = new int64_t;
	delete x;
	auto& btreeAlloc = (*SlabAllocator<BTreeNode>::createSlab());
	btreeTest(btreeAlloc);
	exit(0);
}
