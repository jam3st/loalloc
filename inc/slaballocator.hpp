#pragma once

#include "types.hpp"
#include "sysconfig.hpp"
#include "spinlock.hpp"
#include "debug.hpp"

namespace Gx {
	constexpr size_t CHUNK_SIZE = 512u;
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
			for(;;) {
				if(tmp == 0u) {
					return ret;
				}
				++ret;
				tmp >>= contBitPos;
			}

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
			ssize_t pos;
			size_t len;
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

		struct d { size_t a; size_t f; bool inconsistent; };
		d dump() {
			Debug::start() + "BO: 0x" + header.next + ",0x" + header.prev + ",0x" + header.firstOffset + ",0x" +
			+ header.largestFree + ",0x" + header.pos + ":";
			bool prevAllocation = !decode(0u).val.allocated;
			auto pos = 0u;
			d sums;
			sums.a = 0u;
			sums.f = 0u;
			sums.inconsistent = false;
			auto totalLen = 0u;
			while(pos < count()) {
				auto dec = decode(pos);
				pos += dec.len;
				auto cum = sums.a + sums.f;
				Debug::start() + (dec.val.allocated ? "A" : "F" ) + ",0x" + cum + ",0x" + dec.val.val + ",";
				totalLen += dec.len;
				if(dec.val.allocated) {
					sums.a += dec.val.val;
				} else {
					sums.f += dec.val.val;
				}
				if(!sums.inconsistent && prevAllocation == dec.val.allocated) { sums.inconsistent = true; }
				prevAllocation = dec.val.allocated;
			}
			Debug::start() + "0x" + totalLen + Debug::end;
			if(count() != totalLen) { __builtin_trap(); }
			return sums;
		}

		struct Context {
			DecodedBitmapVal prevVal { .pos = 0u, .len = 0u };
			DecodedBitmapVal currVal { .pos = 0u, .len = 0u };
			DecodedBitmapVal nextVal { .pos = 0u, .len = 0u };
		};

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
			constexpr static size_t values[CHUNK_SIZE/ sizeof(size_t) - 2 * sizeof(size_t)] = {
					defaultAlignmentBytes, 2 * defaultAlignmentBytes, 3 * defaultAlignmentBytes, 4 * defaultAlignmentBytes,
					6 * defaultAlignmentBytes, 8 * defaultAlignmentBytes, 12 * defaultAlignmentBytes, 16 * defaultAlignmentBytes,
                    6 * defaultAlignmentBytes, 8 * defaultAlignmentBytes, 12 * defaultAlignmentBytes, 16 * defaultAlignmentBytes, 0
            };
			uint8_t toReplace[CHUNK_SIZE - -2 * sizeof(size_t)];
		};
		ssize_t totalAlloc;
		size_t allocLength;
		BitmapObjectJumpList jumpList;

        BitmapObject* getNearest(size_t const len) const {
            return reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
        }

        BitmapObject* getFirst() const {
            return reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
        }

		void initMaps(size_t const used, size_t const free) {
			auto first = reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
			first->prev(first);
			first->next(first);
			first->prev()->append(used, true);
			first->prev()->append(free, false);
			totalAlloc += used;
			allocLength += used + free;
		}

		void extend(size_t const size);
		void contract(size_t const size);
	public:
		void deAllocate(void* const what, uint32_t const size) {
			if (size == 0u) { __builtin_trap(); }
			size_t offset = reinterpret_cast<size_t>(what) - reinterpret_cast<size_t>(this);
			auto *last = getFirst()->prev();
			do {
				if(last->remove(offset, size)) {
					totalAlloc -= size;
					if(getFirst()->prev() == last) {
						auto trimSize = last->trimLast(minPageFrameSize, false);
						if(trimSize) {
Debug::start() + "*********************** CONTRACT: " + trimSize + Debug::end;
							contract(trimSize);
						}
					}
					if(totalAlloc < 0) { __builtin_trap(); }
					break;
				}
			} while(last->next() != last);
		}

		void* allocate(uint32_t const size) {
			if(size == 0u) { __builtin_trap(); }
			for(auto* bitmap = getFirst(); ; bitmap = bitmap->next()) {
				auto const found = bitmap->firstFit(size);
				if(found != 0u) {
					totalAlloc += size;
					if(totalAlloc > allocLength) { __builtin_trap(); }
					return reinterpret_cast<void*>(found + reinterpret_cast<size_t>(this));
				}
				if(bitmap == bitmap->next()) {
					break;
				}
			}
			extend(size);
Debug::start() + "*********************** EXTENDED:" + size + Debug::end;
dump();
			auto const realloced = getFirst()->prev()->firstFit(size);
			if(realloced == 0u) { __builtin_trap(); }
		totalAlloc += size;
			return reinterpret_cast<void*>(realloced + reinterpret_cast<size_t>(this));
		}

		void dump() {
			BitmapObject::d sums;
			bool inconsistent = false;
			sums.a = sums.f = 0u;
			sums.inconsistent = false;
			for(auto* bitmap = getFirst(); ; bitmap = bitmap->next()) {
				auto sum = sums.a + sums.f;
				if(sum != bitmap->offset()) { __builtin_trap(); }
				auto tmp = bitmap->dump();
				sums.a += tmp.a;
				sums.f += tmp.f;
				if(!inconsistent && sums.inconsistent) { inconsistent = true; }
				if(bitmap == bitmap->next()) {
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