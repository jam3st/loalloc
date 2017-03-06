#pragma once

#include "types"
#include "bitops"

namespace Gx {
	constexpr size_t cacheLineSize = 64u;
	constexpr size_t minPageBitSize = 12u;
	constexpr size_t minPageFrameSize = 1u << minPageBitSize;
	constexpr size_t minAlignmentBits = 2u;
	constexpr size_t defaultAlignmentBits = 4u;
	constexpr size_t defaultAlignmentBytes = 16u;

	static_assert(defaultAlignmentBits >= minAlignmentBits, "");

	template<size_t padLen>
	struct PadIfNonZero {
		uint8_t pad[padLen];
	};

	template<>
	struct PadIfNonZero<0> {
	};

	template<typename T>
	using PadToAlignment = PadIfNonZero<(sizeof(T) % defaultAlignmentBytes == 0 ? 0 : defaultAlignmentBytes - sizeof(T) % defaultAlignmentBytes)>;

	constexpr inline size_t alignToBits(size_t const val, size_t const alignmentBits) {
		return (val + (1u << alignmentBits) - 1) & ~((1u << alignmentBits) - 1u);
	}

	constexpr inline size_t roundUpNearestMultiple(size_t const val, size_t const alignment) {
		return alignment * ((val + alignment - 1u) / alignment);
	}
	constexpr inline size_t roundDownNearestMultiple(size_t const val, size_t const alignment) {
		return alignment * (val / alignment);
	}
}