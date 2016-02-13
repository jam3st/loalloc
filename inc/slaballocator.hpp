#pragma once

#include "types.hpp"
#include "sysconfig.hpp"
#include "spinlock.hpp"
#include "debug.hpp"

namespace Gx {
	constexpr size_t CHUNK_SIZE = 512u;
	constexpr size_t REBALANCE_THRESHOLD = 18u; // approx 2 x worst case
	constexpr size_t MALLOC_HEADER_SIZE = sizeof(size_t);

	template<size_t size>
	class VarInts final {
	public:
		static constexpr uint8_t signBitPos = 6u;
		static constexpr uint8_t contBitPos = 7u;
		static constexpr uint8_t signBitMask =  1u << signBitPos;
		static constexpr uint8_t contBitMask = 1u << contBitPos;
		static constexpr uint8_t firstPayloadBitMask = ~(signBitMask | contBitMask);
		static constexpr uint8_t payloadBitMask = ~(contBitMask);

	public:
		uint8_t get(size_t const pos) const {
			return data[pos];
		}
		void set(size_t const pos, uint8_t const val) {
			data[pos] = val;
		}

		void shift(ssize_t const shiftLen, size_t const insertPos, size_t const endPos) {
			if(shiftLen == 0) {
				return;
			}
			if(shiftLen > 0 && (shiftLen + insertPos) > size) { __builtin_trap(); }
			if((shiftLen + endPos) > size) { __builtin_trap(); }
			if(shiftLen > 0) {
				for (size_t pos = endPos + shiftLen; pos > insertPos; --pos) {
					data[pos - 1u] = data[pos - shiftLen - 1u];
				}
			} else {
				for (size_t pos = insertPos; pos < endPos + shiftLen; ++pos) {
					data[pos] = data[pos - shiftLen];
				}
			}
		}

		static size_t getLen(uint32_t  const val) {
			size_t ret = 1u;
			auto tmp = val;
			tmp >>= signBitPos;
			while(tmp != 0u) {
				++ret;
				tmp >>= contBitPos;
			}
			return ret;
		}
	private:
		uint8_t data[size];
	};

	class BitmapObject final {
	public:
		struct BitmapVal {
			uint32_t val: sizeof(uint32_t) * bitsPerByte - 1u;
			bool allocated : 1;
		};

		struct  DecodedBitmapVal {
			ssize_t pos = 0;
			size_t len = 0u;
			BitmapVal val;
		};

		DecodedBitmapVal reverseDecode(size_t const endPos) const {
			auto curPos = endPos;
			if(curPos == 0u) {
				return DecodedBitmapVal{ .pos = 0, .len = 0u };
			}

			while(--curPos > 0u) {
				if((v.get(curPos - 1u) & v.contBitMask) == 0u) {
					break;
				}
			}
			return decode(curPos);
		}

		DecodedBitmapVal decode(size_t const startPos) const {
			DecodedBitmapVal currVal;
			if(startPos >= count()) {
				currVal.len = 0u;
				currVal.pos = count();
				currVal.val.allocated = false;
				currVal.val.val = 0u; // used for additions
				return currVal;
			}
			auto curPos = startPos;
			currVal.pos = curPos;
			size_t currByte = v.get(curPos++);
			currVal.len = 1u;
			currVal.val.val = currByte & v.firstPayloadBitMask;
			currVal.val.allocated = (currByte & v.signBitMask) == v.signBitMask;
			size_t currByteBitPos = v.signBitPos;
			while((currByte & v.contBitMask) != 0u) {
				if(curPos > count()) { __builtin_trap(); }
				++currVal.len;
				currByte = v.get(curPos++);
				currVal.val.val |= (currByte & v.payloadBitMask) << currByteBitPos;
				currByteBitPos += v.contBitPos;
			}

			return currVal;
		}

		size_t encode(BitmapVal const val, size_t const insertPos) {
			auto tmp = val.val;
			uint8_t firstByte =  (tmp & v.firstPayloadBitMask) | (val.allocated ? v.signBitMask : 0u) |
					((tmp & ~(v.firstPayloadBitMask)) == 0u ? 0u : v.contBitMask);
			tmp >>= v.signBitPos;
			auto nibIndex = insertPos;
			v.set(nibIndex++, firstByte);
			while(tmp != 0u) {
				v.set(nibIndex++, (tmp & v.payloadBitMask) | ((tmp & ~(v.payloadBitMask)) == 0u ? 0u : v.contBitMask));
				tmp >>= v.contBitPos;
			}
			return nibIndex - insertPos;
		}

		size_t trimLast(size_t const size, bool const allocated) {
			auto lastVal = reverseDecode(count());
			if(lastVal.val.allocated == allocated && lastVal.val.val >= size) {
				count(count() - lastVal.len);
				return lastVal.val.val;
			}
			return 0u;
		}

		BitmapObject* prev() const {
			return header.prev;
		}

		void prev(BitmapObject* const prev) {
			header.prev = prev;
		}


		BitmapObject* next() const {
            return header.next;
        }

		void next(BitmapObject* const next) {
			header.next = next;
		}

		size_t offset() const {
            return header.firstOffset;
        }

		void offset(size_t const offset) {
			header.firstOffset = offset;
		}

		uint32_t largestFree() const {
			return header.largestFree;
		}

		void largestFree(uint32_t const largestFree) {
			header.largestFree = largestFree;
		}

		size_t count() const {
			return header.pos;
		}

		void count(size_t const lastPos) {
			if(lastPos > sizeof(v)) { __builtin_trap(); }
			header.pos = lastPos;
		}

		struct d { size_t a; size_t f; bool lastAlloc; bool inconsistent; };
		d dump(bool prevAllocation);

		struct Context {
			DecodedBitmapVal prevVal { .pos = 0u, .len = 0u };
			DecodedBitmapVal currVal { .pos = 0u, .len = 0u };
			DecodedBitmapVal nextVal { .pos = 0u, .len = 0u };
		};

		bool rebalance();

		size_t findBySize(uint32_t const size);
		bool findByOffset(size_t const globalOffset, uint32_t const len);

		bool remove(size_t const newOffset, uint32_t const len) {
			return findByOffset(newOffset, len);
		}

		void append(size_t const size, bool const allocated) {
			DecodedBitmapVal currVal;
			size_t thisOffset = 0u;
			size_t pos = 0u;
			while(pos < count()) {
				thisOffset += currVal.val.val;
				currVal = decode(pos);
				pos += currVal.len;
			}
			if((currVal.val.allocated && allocated) || (!currVal.val.allocated && !allocated)) {
				BitmapVal newVal;
				newVal.allocated = allocated;
				newVal.val = currVal.val.val + size;
				auto newLen = v.getLen(newVal.val);
				ssize_t shift = newLen - currVal.len;
				v.shift(shift, pos, count());
				encode(newVal, pos - currVal.len);
				count(count() + shift);
			} else {
				BitmapVal newVal;
				newVal.allocated = allocated;
				newVal.val = size;
				// overflow
				auto encLen = encode(newVal, pos);
				count(count() + encLen);
			}
		}

		size_t firstFit(uint32_t const size) {
			return findBySize(size);
		}

	private:

        class BitmapHeader final {
        public:
			PadIfNonZero<MALLOC_HEADER_SIZE> mallocLegacyHeader;
            BitmapObject* next;
            BitmapObject* prev;
            size_t firstOffset;
			uint32_t largestFree;
            uint16_t pos;
			SpinIncrementLock16 lock;
        };

		BitmapHeader header;
		VarInts<roundUpNearestMultiple(CHUNK_SIZE, cacheLineSize) - sizeof(header)> v;
	};

	class Bitmaps final {
	private:
		friend class BitmapObject;
		class BitmapObjectJumpList {
//			constexpr static size_t values[CHUNK_SIZE/ sizeof(size_t) - 2 * sizeof(size_t)] = {
//					defaultAlignmentBytes, 2 * defaultAlignmentBytes, 3 * defaultAlignmentBytes, 4 * defaultAlignmentBytes,
//					6 * defaultAlignmentBytes, 8 * defaultAlignmentBytes, 12 * defaultAlignmentBytes, 16 * defaultAlignmentBytes,
//                    6 * defaultAlignmentBytes, 8 * defaultAlignmentBytes, 12 * defaultAlignmentBytes, 16 * defaultAlignmentBytes, 0
//            };
//			uint8_t toReplace[CHUNK_SIZE - sizeof(ssize_t) - sizeof(size_t) - sizeof(BitmapObject*)];
		};
		ssize_t totalAlloc;
		size_t allocLength;
		BitmapObject* spare;
		BitmapObjectJumpList jumpList;

        BitmapObject* getNearest(size_t const len) const {
            return reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
        }

        BitmapObject* first() const {
            return reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
        }

		BitmapObject* last() const {
			return reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps))->prev();
		}

		void initMaps(size_t const used, size_t const free) {
			auto first = reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
			first->prev(first);
			first->next(first);
			first->prev()->append(used, true);
			first->prev()->append(free, false);
			totalAlloc += used;
			allocLength += used + free;
			spare = reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(BitmapObject) + sizeof(Bitmaps));
			auto xx = (size_t*)spare;
			for(int i =0; i < sizeof(*spare)/sizeof(size_t); ++i) {
				if(xx[i] != 0ULL) {
					__builtin_trap();
				}
				xx[i] = (size_t)spare | (sizeof(*spare) << 32);
			}
		}
		BitmapObject* getSpare(){
			if(spare == nullptr) { __builtin_trap(); }
			auto ret = spare;
			spare = nullptr;
			return ret;
		}
		void extend(size_t const size);
		void contract(size_t const size);
		void allocateSpare() {
			if(spare == nullptr) {
Debug::start() + "*********************** ALLOC SPARE: " + Debug::end;
				spare = reinterpret_cast<BitmapObject*>(allocate(sizeof(*spare), false));
				auto xx = (size_t*)spare;
				for(int i =0; i < sizeof(*spare)/sizeof(size_t); ++i) {
					if(xx[i] != 0ULL) {
						dump();
						__builtin_trap();
					}
					xx[i] = (size_t)spare | sizeof(*spare) << 32;
				}
Debug::start() + "*********************** ALLOC DONE: " + Debug::end;
			}
		}
	public:
		void deAllocate(void* const what, uint32_t const size) {
			if (size == 0u) { __builtin_trap(); }
			auto allocSize = alignToBits(size, 4);
			size_t offset = reinterpret_cast<size_t>(what) - reinterpret_cast<size_t>(this);
			for (auto curr = Bitmaps::allocator->first()->prev(); ; ) {
				if(curr->remove(offset, allocSize)) {
					totalAlloc -= allocSize;
					if(first()->prev() == curr) {
						auto trimSize = curr->trimLast(minPageFrameSize, false);
						if(trimSize) {
Debug::start() + "*********************** CONTRACT: " + trimSize + Debug::end;
							contract(trimSize);
						}
					}
					if(totalAlloc < 0) { __builtin_trap(); }
					break;
				}
				curr = curr->prev();
				if (curr == Bitmaps::allocator->first()->prev()) {
					Debug::start() + "*********************** deAllocate: " + Debug::end;
					__builtin_trap();
				}
			}
			allocateSpare();
		}

		void* allocate(uint32_t const size, bool const spare = true) {
			if(size == 0u) { __builtin_trap(); }
			auto allocSize = alignToBits(size, 4);
			size_t found = 0u;
			for(size_t maxTries = 0; found == 0u && maxTries < 2; ++maxTries) {
				for (auto *bitmap = first(); ; ) {
					found = bitmap->firstFit(allocSize);
					if (found != 0u) {
						totalAlloc += allocSize;
						if (totalAlloc > allocLength) { __builtin_trap(); }
						break;
					}
					bitmap = bitmap->next();
					if (bitmap == first()) {
						break;
					}
				}
				if(found == 0u && maxTries == 0u) {
					extend(size);
					Debug::start() + "*********************** EXTENDED:" + size + Debug::end;
				}
				if(spare) {
					allocateSpare();
				}

			}
			return reinterpret_cast<void*>(found + reinterpret_cast<size_t>(this));
		}

		void dump() {
			BitmapObject::d sums;
			bool inconsistent = false;
			sums.a = sums.f = 0u;
			sums.inconsistent = false;
			bool prevAllocation = false;


			for(auto* bitmap = first(); ; ) {
				auto const sum = sums.a + sums.f;
				if(sum != bitmap->offset()) { inconsistent = true;  Debug::start() + "Summed 0x" + sum + " Offset is 0x" + bitmap->offset() + Debug::end; }
				auto tmp = bitmap->dump(prevAllocation);
				sums.a += tmp.a;
				sums.f += tmp.f;
				if(!inconsistent && sums.inconsistent) { inconsistent = true; }
				prevAllocation = sums.lastAlloc;
				bitmap = bitmap->next();
				if(bitmap == first()) {
					break;
				}
			}
			auto cum = sums.a + sums.f;
			Debug::start() + "Alloc len is " + allocLength + " vs summed " + cum + " and useds is " + totalAlloc + " vs summs " + sums.a + Debug::end;
			if(sums.a + sums.f != allocLength) { __builtin_trap(); }
			if(sums.a != totalAlloc)  { __builtin_trap(); }
			if(inconsistent)  { __builtin_trap(); }
		};

		static void init();
		static Bitmaps* allocator;
	};
}