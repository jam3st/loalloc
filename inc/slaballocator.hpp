#pragma once

#include "types.hpp"
#include "sysconfig.hpp"

namespace Gx {
	template<typename T, typename U>
	constexpr auto offsetOf(U const T::*member) {
		return reinterpret_cast<size_t >(&(reinterpret_cast<T*>(0u)->*member));
	}

	class BaseAllocator {
	private:
		explicit BaseAllocator() = delete;
	public:
		typedef size_t bitmapType;
	protected:
		static BaseAllocator* createSlab(size_t const objSize, size_t const slabSize);
		void* alloc(size_t const objSize);
		void dealloc(size_t const objSize, void* const what);
	private:
		static constexpr size_t counterBitsSize = sizeof(uint32_t) * bitsPerByte;
		static constexpr size_t bitsPerAllocation = 22u;
		static constexpr size_t bitsForBitmapLen = bitsPerAllocation - defaultAlignmentBits - bitsPerByte;
		static constexpr size_t bitsForCacheCount = 31;
		static constexpr size_t invalidCacheCount = (1u << bitsForCacheCount) - 1;
	public:
		static constexpr size_t maxAllocLen = (1u << bitsPerAllocation);
	private:
		static_assert(counterBitsSize == bitsPerAllocation + bitsForBitmapLen, "Go back to the drawing board");
		static_assert(counterBitsSize >= bitsForCacheCount + 1, "Go back to the drawing board");
		BaseAllocator* createSlab() const;
		size_t getCount(size_t const objSize);
		size_t headerLen() const {
			return header.bitmapSize * sizeof(bitmap[0]) + offsetOf(&BaseAllocator::header);
		};
		
		size_t numberOfElements(size_t const objSize) const;
		size_t offsetStartPointers() const;

		void* allocFromSlab(size_t const objSize);
		void deAllocFromSlab(void* const what, size_t const objSize);

	protected:
	public:
		class Header  {
		public:
			BaseAllocator* next;
			uint32_t bitmapSize : bitsForBitmapLen;
			uint32_t allocLen : bitsForBitmapLen;
			bool full : (counterBitsSize - bitsForCacheCount);
			uint32_t cacheCount : bitsForCacheCount;
		} header;

		alignas(defaultAlignmentBytes) bitmapType bitmap[1];
	};

	template<typename T>
	class SlabAllocator final : public BaseAllocator {
	public:

		explicit SlabAllocator() = delete;
		static SlabAllocator<T>* createSlab(size_t const slabSize = maxAllocLen) {
			auto const actualSize = (slabSize > maxAllocLen) ? maxAllocLen :  slabSize;
			return static_cast<SlabAllocator<T>*>(BaseAllocator::createSlab(sizeof(T), actualSize));
		}
		T* alloc() {
			return static_cast<T*>(BaseAllocator::alloc(sizeof(T)));
		}

		void dealloc(T* const what) {
			BaseAllocator::dealloc(sizeof(T), what);
		}

	private:
//	private:
//		PadIfNonZero<(sizeof(BaseAllocator) % cacheLineSize == 0 ? 0  : cacheLineSize - sizeof(BaseAllocator) % cacheLineSize)> pad0;
	};


}