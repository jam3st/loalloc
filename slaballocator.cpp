#include "regionallocator"

namespace Gx {
    void BitmapObject::offset(size_t const offset) {
        if(Bitmaps::allocator->first() == this && offset !=0u) {
            __builtin_trap();
        }
        header.firstOffset = offset;
    }

    void *initBrk();

    size_t extendBrk(void *const alloc, size_t const len);

    Bitmaps *Bitmaps::allocator = nullptr;

    void Bitmaps::init() {
        allocator = reinterpret_cast<Bitmaps *>(initBrk());
        constexpr size_t initialUsed = alignToBits(sizeof(Bitmaps) + sizeof(BitmapObject), alignmentBits);
        constexpr size_t initialAlloc = roundUpNearestMultiple(initialUsed, minPageFrameSize);
        if (extendBrk(allocator, initialAlloc) != initialAlloc) { __builtin_trap(); }
        allocator->initMaps(initialUsed, initialAlloc - initialUsed);
        allocator->dump();
    }

    void Bitmaps::extend(size_t const size) {
        auto extensionSize = roundUpNearestMultiple(size, minPageFrameSize);
        if (allocLength + extensionSize == allocLength) { __builtin_trap(); }
        allocLength += extensionSize;
        if (extendBrk(allocator, allocLength) != allocLength) { __builtin_trap(); }
        first()->prev()->append(extensionSize, false);
    }

    void Bitmaps::contract(size_t const size) {
        auto contractionSize = roundDownNearestMultiple(size, minPageFrameSize);
        allocLength -= contractionSize;
        if (extendBrk(allocator, allocLength) != allocLength) { __builtin_trap(); }
        if (size < contractionSize) { __builtin_trap(); }
        if (size != contractionSize) {
            first()->prev()->append(size - contractionSize, false);
        }
    }

    void Bitmaps::initMaps(size_t const used, size_t const free) {
        auto first = reinterpret_cast<BitmapObject*>(reinterpret_cast<size_t>(this) + sizeof(Bitmaps));
        first->prev(first);
        first->next(first);
        first->prev()->append(used, true);
        first->prev()->append(free, false);
        totalAlloc += used;
        allocLength += used + free;
        spare = reinterpret_cast<BitmapObject*>(allocate(sizeof(BitmapObject), false));
    }

    void BitmapObject::append(size_t const sizeofSize, bool const allocated) {
        const size_t size = alignToBits(sizeofSize, alignmentBits) >> alignmentBits;
        DecodedBitmapVal lastVal = reverseDecode(count());;
        if((lastVal.val.allocated && allocated) || (!lastVal.val.allocated && !allocated)) {
            BitmapVal newVal;
            newVal.allocated = allocated;
            newVal.val = lastVal.val.val + size;
            auto newLen = v.getLen(newVal.val);
            ssize_t shift = newLen - lastVal.len;
            encode(newVal, lastVal.pos);
            count(count() + shift);
        } else {
            BitmapVal newVal;
            newVal.allocated = allocated;
            newVal.val = size;
            // overflow
            auto encLen = encode(newVal, count());
            count(count() + encLen);
        }
    }

    size_t BitmapObject::trimLast(size_t const size, bool const allocated) {
        auto lastVal = reverseDecode(count());
        if(lastVal.val.allocated == allocated && lastVal.val.val >= size) {
            count(count() - lastVal.len);
            return lastVal.val.val << alignmentBits;
        }
        return 0u;
    }

    BitmapObject::d BitmapObject::dump(bool prevAllocation, bool const lucid) {
        if(lucid) {
            Debug::start() + "BO: 0x" + this + ",0x" + header.prev + ",0x" + header.next + ",0x" + header.firstOffset +
            ",0x" +
            +header.largestFree + ",0x" + header.pos + ":";
        }
        auto pos = 0u;
        d sums;
        sums.a = 0u;
        sums.f = 0u;
        sums.inconsistent = false;
        auto totalLen = 0u;
        auto thisOffset = offset();
        while(pos < count()) {
            auto dec = decode(pos);
            pos += dec.len;
            auto cum = sums.a + sums.f;
            if(lucid) {
                Debug::start() + (dec.val.allocated ? "A" : "F") + ",0x" + cum + ",0x" + (dec.val.val << alignmentBits) + ",";
            }
            totalLen += dec.len;
            if(dec.val.allocated) {
                sums.a += dec.val.val << alignmentBits;
            } else {
                sums.f += dec.val.val << alignmentBits;
            }
            thisOffset += dec.val.val << alignmentBits;
            if(!sums.inconsistent && prevAllocation != dec.val.allocated) { sums.inconsistent = true; }
            prevAllocation = dec.val.allocated;
        }
        if(lucid) {
            Debug::start() + "0x" + totalLen + Debug::end;
        }
        if(count() > sizeof(v)) { __builtin_trap(); }
        if(count() != totalLen) { __builtin_trap(); }
        sums.lastAlloc = prevAllocation;
        return sums;
    }

    void BitmapObject::rebalance() {
        constexpr size_t mergeThreshold = sizeof(v) / 2 -  REBALANCE_THRESHOLD;
        if (count() + REBALANCE_THRESHOLD > sizeof(v)) {
            if((prev()->count() >= mergeThreshold && next()->count() >= mergeThreshold)
                || prev() == Bitmaps::allocator->first() ||
                   next() == Bitmaps::allocator->first()) {
                auto spare = Bitmaps::allocator->getSpare();
                spare->next(next());
                spare->prev(this);
                spare->next()->prev(spare);
                next(spare);
                size_t currOffset = offset();
                DecodedBitmapVal val;
                while(val.pos < count() / 2) {
                    val = decode(val.pos + val.len);
                    if(val.val.val == 0u) { __builtin_trap(); }
                    currOffset += (val.val.val << alignmentBits);
                }
                auto split = val.pos + val.len;
                auto sparePos = split;
                while(sparePos < count()) {
                    spare->v.set(v.get(sparePos), sparePos - split);
                    ++sparePos;
                }
                count(split);
                spare->offset(currOffset);
                spare->count(sparePos - split);
            } else if(this != Bitmaps::allocator->first() && prev()->count() < mergeThreshold) {
                size_t diffOffset = 0u;
                DecodedBitmapVal val;
                while((val.pos + val.len) < count() / 2) {
                    val = decode(val.pos + val.len);
                    if(val.val.val == 0u) { __builtin_trap(); }
                    diffOffset += (val.val.val << alignmentBits);
                }
                // use memmov
                auto numToCopy = val.pos + val.len;
                auto startPos = prev()->count();
                for(auto i = 0u; i < numToCopy; ++i) {
                    prev()->v.set(v.get(i), startPos + i);
                }
                v.shift(-numToCopy, 0u, count());
                count(count() - numToCopy);
                prev()->count(prev()->count() + numToCopy);
                offset(offset() + diffOffset);
            } else if(next() != Bitmaps::allocator->first() && next()->count() < mergeThreshold) {
                size_t diffOffset = 0u;
                DecodedBitmapVal val;
                val = reverseDecode(count() / 2);
                auto startPos = val.pos + val.len;
                while((val.pos + val.len) < count()) {
                    val = decode(val.pos + val.len);
                    if(val.val.val == 0u) { __builtin_trap(); }
                    diffOffset += (val.val.val << alignmentBits);
                }
                // use memmov
                auto numToCopy = count() - startPos;
                next()->v.shift(numToCopy, 0, next()->count());
                for(auto i = 0u; i < numToCopy; ++i) {
                    next()->v.set(v.get(i + startPos), i);
                }
                count(count() - numToCopy);
                next()->count(next()->count() + numToCopy);
                next()->offset(next()->offset() - diffOffset);
            }
        } else if (this != Bitmaps::allocator->first() && count() < REBALANCE_THRESHOLD) {
            if(prev()->count() + count() + REBALANCE_THRESHOLD < sizeof(v)) {
                auto numToCopy = count();
                auto startPos = prev()->count();
                for(auto i = 0u; i < numToCopy; ++i) {
                    prev()->v.set(v.get(i), startPos + i);
                }
                prev()->count(prev()->count() + numToCopy);
                prev()->next(next());
                next()->prev(prev());
                Bitmaps::allocator->toDelete = this;
            }  else if(next() != Bitmaps::allocator->first() && next()->count() + count() + REBALANCE_THRESHOLD < sizeof(v)) {
                auto numToCopy = count();
                next()->v.shift(numToCopy, 0, next()->count());
                for(auto i = 0u; i < numToCopy; ++i) {
                    next()->v.set(v.get(i), i);
                }
                next()->offset(offset());
                next()->count(next()->count() + numToCopy);
                prev()->next(next());
                next()->prev(prev());
                Bitmaps::allocator->toDelete = this;
            }
        }
    }

    BitmapObject::BitmapVal BitmapObject::findBySize(size_t const sizeofSize) {
        const size_t size = alignToBits(sizeofSize, alignmentBits) >> alignmentBits;
        BitmapVal ret;
        Context con;
        ret.val = 0u;
        size_t thisOffset = 0u;
        size_t preMergeOffset = offset();
        do {
            con.prevVal = con.currVal;
            thisOffset += con.currVal.val.val << alignmentBits;
            con.currVal = decode(con.currVal.pos + con.currVal.len);
        } while ((size > con.currVal.val.val || con.currVal.val.allocated) && con.currVal.pos < count());

        if (con.currVal.len == 0u || con.currVal.val.allocated || con.currVal.val.val < size) {
            ret.allocated = false;
            ret.val = 0u;
            return ret;
        }

        if (count() + REBALANCE_THRESHOLD > sizeof(v)) {
            ret.allocated = false;
            ret.val = 1u;
            return ret;
        }


        bool borrowedPrev = false;
        if(con.currVal.pos == 0 && this != Bitmaps::allocator->first() ) {
            borrowedPrev = true;
            con.prevVal = prev()->reverseDecode(prev()->count());
        }
        bool borrowedNext = false;
        if(con.currVal.pos + con.currVal.len == count() && this != Bitmaps::allocator->last()) {
            borrowedNext = true;
            con.nextVal = next()->decode(0u);
        } else {
            con.nextVal = decode(con.currVal.pos + con.currVal.len);
        }

        if (con.currVal.val.allocated || !con.prevVal.val.allocated ||
            (con.nextVal.len != 0u && !con.nextVal.val.allocated)) {
            __builtin_trap();
        }

        if (con.currVal.val.val > size) {
            //split
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = size + con.prevVal.val.val;
            BitmapVal mergedVal;
            mergedVal.allocated = false;
            mergedVal.val = con.currVal.val.val - size;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(mergedVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            if(!borrowedPrev) {
                shiftLen -= con.prevVal.len;
                prevInsertPos -= con.prevVal.len;
            }

            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(newVal, prevInsertPos);
            encLen += encode(mergedVal, prevInsertPos + encLen);
            ssize_t countDiff = encLen - con.currVal.len;

            if(borrowedPrev) {
                prev()->count(prev()->count() - con.prevVal.len);
                offset(offset() - (con.prevVal.val.val << alignmentBits));
            } else {
                countDiff -= con.prevVal.len;
            }
            count(count() + countDiff);

        } else if (con.currVal.val.val == size) {
            // merge with prev
            BitmapVal mergedVal;
            mergedVal.allocated = true;
            mergedVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(mergedVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            if(!borrowedPrev) {
                shiftLen -= con.prevVal.len;
                prevInsertPos -= con.prevVal.len;
            }
            if(!borrowedNext) {
                shiftLen -= con.nextVal.len;
            }

            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(mergedVal, prevInsertPos);
            ssize_t countDiff = encLen - con.currVal.len;

            if(borrowedPrev) {
                prev()->count(prev()->count() - con.prevVal.len);
                offset(offset() - (con.prevVal.val.val << alignmentBits));
            } else {
                countDiff -= con.prevVal.len;
            }
            if(borrowedNext) {
                next()->v.shift(-con.nextVal.len, 0, next()->count());
                next()->count(next()->count() - con.nextVal.len);
                next()->offset(next()->offset() + (con.nextVal.val.val << alignmentBits));
            } else {
                countDiff -= con.nextVal.len;
            }
            count(count() + countDiff);

        }
        ret.val = thisOffset + preMergeOffset;
        ret.allocated = true;
        return ret;
    }

    BitmapObject::FindType BitmapObject::findByOffset(size_t const globalOffset, size_t const sizeofSize) {
        if (globalOffset < offset()) {
            return NotFound;
        }
        if (count() + REBALANCE_THRESHOLD > sizeof(v)) {
            return FoundButNoSpace;
        }

        size_t relativeOffset = globalOffset - offset();
        Context con;
        size_t cumulativeOffset = 0u;
        while (relativeOffset >= cumulativeOffset && con.currVal.pos < count()) {
            con.prevVal = con.currVal;
            con.currVal = decode(con.currVal.pos + con.currVal.len);
            cumulativeOffset += (con.currVal.val.val << alignmentBits);
        }

        if(relativeOffset >= cumulativeOffset) {
            Bitmaps::allocator->dump();
            return NeverGoingToBeFound;
        }

        bool borrowedPrev = false;
        if(con.currVal.pos == 0 && this != Bitmaps::allocator->first() ) {
            borrowedPrev = true;
            con.prevVal = prev()->reverseDecode(prev()->count());
        }
        bool borrowedNext = false;
        if(con.currVal.pos + con.currVal.len == count() && this != Bitmaps::allocator->last()) {
            borrowedNext = true;
            con.nextVal = next()->decode(0u);
        } else {
            con.nextVal = decode(con.currVal.pos + con.currVal.len);
        }
        const size_t size = alignToBits(sizeofSize, alignmentBits) >> alignmentBits;
        const size_t diffSet = (cumulativeOffset - relativeOffset) >>  alignmentBits;
        if (diffSet == con.currVal.val.val && size == con.currVal.val.val) {
            BitmapVal mergedVal;
            mergedVal.allocated = false;
            mergedVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(mergedVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            if(!borrowedPrev) {
                shiftLen -= con.prevVal.len;
                prevInsertPos -= con.prevVal.len;
            }
            if(!borrowedNext) {
                shiftLen -= con.nextVal.len;
            }

            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(mergedVal, prevInsertPos);
            ssize_t countDiff = encLen - con.currVal.len;

            if(borrowedPrev) {
                prev()->count(prev()->count() - con.prevVal.len);
                offset(offset() - (con.prevVal.val.val << alignmentBits));
            } else {
                countDiff -= con.prevVal.len;
            }
            if(borrowedNext) {
                next()->v.shift(-con.nextVal.len, 0, next()->count());
                next()->count(next()->count() - con.nextVal.len);
                next()->offset(next()->offset() + (con.nextVal.val.val << alignmentBits));
            } else {
                countDiff -= con.nextVal.len;
            }
            count(count() + countDiff);
        } else if (diffSet == con.currVal.val.val) {
            BitmapVal prevVal;
            prevVal.allocated = false;
            prevVal.val = size + con.prevVal.val.val;
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.currVal.val.val - size;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(prevVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            if(!borrowedPrev) {
                shiftLen -= con.prevVal.len;
                prevInsertPos -= con.prevVal.len;
            }

            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(prevVal, prevInsertPos);
            encLen += encode(newVal, prevInsertPos + encLen);
            ssize_t countDiff = encLen - con.currVal.len;

            if(borrowedPrev) {
                prev()->count(prev()->count() - con.prevVal.len);
                offset(offset() - (con.prevVal.val.val << alignmentBits));
            } else {
                countDiff -= con.prevVal.len;
            }
            count(count() + countDiff);
        } else if (diffSet == size) {
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.currVal.val.val - size;
            BitmapVal mergedNextVal;
            mergedNextVal.allocated = false;
            mergedNextVal.val = con.nextVal.val.val + size;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(mergedNextVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            if(!borrowedNext) {
                shiftLen -= con.nextVal.len;
            }
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(newVal, prevInsertPos);
            encLen += encode(mergedNextVal, prevInsertPos + encLen);
            ssize_t countDiff = encLen - con.currVal.len;
            if(borrowedNext) {
                next()->v.shift(-con.nextVal.len, 0, next()->count());
                next()->count(next()->count() - con.nextVal.len);
                next()->offset(next()->offset() + (con.nextVal.val.val << alignmentBits));
            } else {
                countDiff -= con.nextVal.len;
            }
            count(count() + countDiff);
        } else {
            BitmapVal prevVal;
            prevVal.allocated = true;
            prevVal.val = con.currVal.val.val - diffSet;
            BitmapVal newVal;
            newVal.allocated = false;
            newVal.val = size;
            BitmapVal nextVal;
            nextVal.allocated = true;
            nextVal.val = diffSet - size;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(prevVal.val) + v.getLen(nextVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(prevVal, prevInsertPos);
            encLen += encode(newVal, prevInsertPos + encLen);
            encLen += encode(nextVal, prevInsertPos + encLen);
            count(count() + encLen - con.currVal.len);
        }
        return Found;
    }
}