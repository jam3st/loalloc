#pragma once

#include "types.hpp"
#include "bitops.hpp"

namespace Gx {
	constexpr size_t cacheLineSize = 64u;
	constexpr size_t minPageBitSize = 12u;
	constexpr size_t minPageFrameSize = 1u << minPageBitSize;
	constexpr size_t defaultAlignmentBits = 4u;
	constexpr size_t defaultAlignmentBytes = 16u;

	template<size_t padLen>
	struct PadIfNonZero {
		uint8_t pad[padLen];
	};
	template<>
		struct PadIfNonZero<0> {
	};

	template<typename T>
	using PadToAlignment = PadIfNonZero<(sizeof(T) % defaultAlignmentBytes == 0 ? 0  : defaultAlignmentBytes - sizeof(T) % defaultAlignmentBytes)>;

	inline size_t alignToBits(size_t const val, size_t const alignmentBits) {
		return (val + ((1u << alignmentBits) - 1u)) & (~((1u << alignmentBits) - 1u));
	}

//PadIfNonZero<(sizeof(HeapBreak) % cacheLineSize == 0 ? 0  : cacheLineSize - sizeof(HeapBreak) % cacheLineSize)> pad0;
}