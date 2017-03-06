#pragma once

namespace Gx {
	inline void *mmap(void *const addr, size_t const len) {
		constexpr size_t sysMmap = 0x40000000u + 9;
		constexpr size_t protRead = 0x00000001u;
		constexpr size_t protWrite = 0x00000002u;
		constexpr size_t flagsPrivate = 0x00000002u;
		constexpr size_t flagsAnon = 0x00000020u;
		constexpr size_t prot = protRead | protWrite;
		constexpr size_t flags = flagsPrivate | flagsAnon;
		constexpr size_t fd = UINT32_MAX;
		constexpr size_t offset = 0;
		void *res;
		__asm__ __volatile__("movl %5, %%r10d;"
			"movl %6, %%r8d;"
			"movl %7, %%r9d;"
			"syscall;" : "=a"(res) : "a"(sysMmap), "D"(addr), "S"(len), "d"(prot), "m"(flags), "m"(fd), "m"(offset) : "r10", "r8", "r9");
		return res;
	}

	inline void *mummap(void *const addr, size_t const len) {
		constexpr size_t sysMmap = 0x40000000u + 11;
		void *res;
		__asm__ __volatile__("syscall;" : "=a"(res) : "a"(sysMmap), "D"(addr), "S"(len)  : );
		return res;
	}

}