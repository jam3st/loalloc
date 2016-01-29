#include "slaballocator.hpp"
#include "syscall.hpp"
#include "bitops.hpp"
#include "sysconfig.hpp"

namespace Gx {

	size_t calculateBitmapSize(size_t const objSize, size_t const len, size_t const netHeaderSize) {
		auto prevBitmapLen = 1u;
		auto newBitmapLen = 0u;
		for(auto iter = 32u; iter != 0; --iter) {
			auto const objSizeAligned = alignToBits(objSize, defaultAlignmentBits);
			auto const maxPossibleObjects = (len - netHeaderSize - sizeof(BaseAllocator::bitmapType) * prevBitmapLen * bitsPerByte ) / objSizeAligned;
			newBitmapLen = maxPossibleObjects / (sizeof(BaseAllocator::bitmapType) * bitsPerByte);
			if(maxPossibleObjects % (sizeof(BaseAllocator::bitmapType) * bitsPerByte) != 0) {
				++newBitmapLen;
			}
			if(prevBitmapLen == newBitmapLen) {
				break;
			}
			if(newBitmapLen != prevBitmapLen) {
				prevBitmapLen = newBitmapLen;
			}
		}
		if(prevBitmapLen < newBitmapLen) {
			prevBitmapLen = newBitmapLen;
		}

		return prevBitmapLen;
	}

	size_t BaseAllocator::numberOfElements(size_t const objSize) const {
		auto const objSizeAligned = alignToBits(objSize, defaultAlignmentBits);
		return (header.allocLen - headerLen()) / objSizeAligned;
	}

	size_t BaseAllocator::offsetStartPointers() const {
		return alignToBits(headerLen(), defaultAlignmentBits);
	}


	BaseAllocator* BaseAllocator::createSlab(size_t const objSize, size_t const len) {
		auto alignedLen = alignToBits(len, minPageBitSize);
		BaseAllocator *ret = static_cast<BaseAllocator *>(mmap(nullptr, alignedLen));
		ret->header.allocLen = alignedLen - 1;
		ret->header.bitmapSize = calculateBitmapSize(objSize, alignedLen, offsetOf(&BaseAllocator::bitmap));
		ret->header.full = false;
		ret->header.cacheCount = 0u;
		return ret;
	}

	BaseAllocator* BaseAllocator::createSlab() const {
		BaseAllocator *ret = static_cast<BaseAllocator *>(mmap(nullptr, header.allocLen + 1));
		ret->header.allocLen = header.allocLen;
		ret->header.bitmapSize = header.bitmapSize;
		ret->header.full = false;
		ret->header.cacheCount = 0u;
		return ret;
	}

	void* BaseAllocator::alloc(size_t const objSize) {
		auto prev = this;
		for(auto curr = prev; curr != nullptr; curr = curr->header.next) {
			auto ret = curr->allocFromSlab(objSize);
			if(ret != nullptr) {
				return ret;
			}
			prev = curr;
		}
		prev->header.next = createSlab();
		return prev->header.next->allocFromSlab(objSize);
	}

	void BaseAllocator::deAllocFromSlab(void* const what, size_t const objSize) {
		auto offset = reinterpret_cast<uintptr_t>(what) - reinterpret_cast<uintptr_t>(this);
		if(what > this && offset < minPageFrameSize) {
			auto slabPos = (offset - offsetStartPointers()) / alignToBits(objSize, defaultAlignmentBits);
			auto bitmapIndex = slabPos / (bitsPerByte * sizeof(bitmapType));
			auto bitPos = slabPos % (bitsPerByte * sizeof(bitmapType));
			bitmap[bitmapIndex] &= ~(static_cast<bitmapType>(1u) << bitPos);
			header.full = false;
			header.cacheCount = invalidCacheCount;
		}
	}

	void *BaseAllocator::allocFromSlab(size_t const objSize) {
		if(header.full) {
			return nullptr;
		}
		void* ret(nullptr);
		for (auto i = 0u; i < header.bitmapSize; ++i) {
			auto bitPos = Bitops::firstBitSet(~bitmap[i]);
			auto slabPos = bitPos + i * bitsPerByte * sizeof(bitmapType);
			if (bitPos < bitsPerByte * sizeof(bitmap[0]) && slabPos < numberOfElements(objSize)) {
				bitmap[i] |= static_cast<bitmapType>(1u) << bitPos;
				auto thisPtr = reinterpret_cast<uintptr_t>(this);
				// 1 base
				auto offset = slabPos * alignToBits(objSize, defaultAlignmentBits)
				              + offsetStartPointers();
				ret = reinterpret_cast<void *>(thisPtr + offset);
			}
			if(slabPos == (numberOfElements(objSize) - 1)) {
				header.full = true;
				header.cacheCount = invalidCacheCount;
			}
		}
		return ret;
	}

	void BaseAllocator::dealloc(size_t const objSize, void* const what) {
		if(what == nullptr) {
			return;
		}
		auto secondLast = this;
		auto last = this;
		for(auto curr = this; curr != nullptr; curr = curr->header.next) {
			curr->deAllocFromSlab(what, objSize);
			if(curr->header.next == nullptr) {
				last = curr;
			} else {
				secondLast = curr;
			}
		}
		if(secondLast != last && last->header.next == nullptr && last->getCount(objSize) == 0 && secondLast->getCount(objSize) < ((numberOfElements(objSize) / 2) + 1)) {
			if(secondLast->header.next != last ) __builtin_trap();
			if(last->header.next != nullptr ) __builtin_trap();
			mummap(secondLast->header.next, header.allocLen);
			secondLast->header.next = nullptr;
		}
	}

	size_t BaseAllocator::getCount(size_t const objSize) {
		if(header.cacheCount == invalidCacheCount) {
			auto tmp = 0u;
			for (auto i = 0u; i < header.bitmapSize; ++i) {
				for (auto curr = bitmap[i]; curr != 0; curr &= curr - 1) {
					++tmp;
				}
			}
			header.cacheCount = tmp;
		}
		return header.cacheCount;
	}
}
