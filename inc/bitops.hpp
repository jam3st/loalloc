#pragma once

#include "types.hpp"

namespace Gx {
	namespace Bitops {
		inline size_t firstBitSet(uint32_t val)
		{
			if(val == 0u) {
				return bitsPerByte * sizeof(val);
			}
			uint32_t bitPos;
			__asm__ ("bsfl %[val], %[bitPos];"
				: [bitPos] "=r"(bitPos)
				: [val] "r"(val)
				: "cc");
			return bitPos;
		}

	}
}