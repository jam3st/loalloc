#pragma once

#include "types.hpp"

namespace Gx {
	namespace Atomic {
		inline uint8_t Load(uint8_t const& src) {
			uint8_t res;
			__asm__ __volatile__("movb %1, %0;"
					  : "=r" (res)
			                      : "m" (src)
					  : "memory"
			);
			return res;
		}

		inline uint16_t Load(uint16_t const& src) {
			uint16_t res;
			__asm__ __volatile__("movw %1, %0;"
					  : "=r" (res)
					  : "m" (src)
					  : "memory"
			);
			return res;
		}

		inline uint32_t Load(uint32_t const& src) {
			uint32_t res;
			__asm__ __volatile__("movl %1, %0;"
					 : "=r" (res)
					 : "m" (src)
					 : "memory"
			);
			return res;
		}

		inline uint32_t Load(uint64_t const& src) {
			uint64_t res;
			__asm__ __volatile__("movq %1, %0;"
					 : "=r" (res)
			                     : "m" (src)
					 : "memory"
			);
			return res;
		}

		inline bool CompareAndSet(uint8_t& dst, uint8_t const curr, uint8_t const val) {
			uint8_t retVal;
			__asm__ __volatile__("lock cmpxchgb %2, %1;"
					 : "=a" (retVal), "+m" (dst)
 					 : "q" (val), "0" (curr)
					 : "memory", "cc"
			);
			return curr == retVal;
		}

		inline bool CompareAndSet(uint16_t& dst, uint16_t const curr, uint16_t const val) {
			uint16_t retVal;
			__asm__ __volatile__("lock cmpxchgw %2, %1;"
			                     : "=a" (retVal), "+m" (dst)
					 : "q" (val), "0" (curr)
					 : "memory", "cc"
			);
			return curr == retVal;
		}

		inline bool CompareAndSet(uint32_t& dst, uint32_t const curr, uint32_t const val) {
			uint32_t retVal;
			__asm__ __volatile__("lock cmpxchgl %2, %1;"
					 : "=a" (retVal), "+m" (dst)
					 : "q" (val), "0" (curr)
					 : "memory", "cc"
			);
			return curr == retVal;
		}

		inline bool CompareAndSet(uint64_t& dst, uint64_t const curr, uint64_t const val) {
			uint64_t retVal;
			__asm__ __volatile__("lock cmpxchgq %2, %1;"
					 : "=a" (retVal), "+m" (dst)
					 : "q" (val), "0" (curr)
					 : "memory", "cc"
			);
			return curr == retVal;
		}

	}
}