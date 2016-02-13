#include "slaballocator.hpp"

namespace Gx {

    void *initBrk();

    size_t extendBrk(void *const alloc, size_t const len);

    Bitmaps *Bitmaps::allocator = nullptr;

    void Bitmaps::init() {
        allocator = reinterpret_cast<Bitmaps *>(initBrk());
        constexpr size_t initialUsed = sizeof(Bitmaps) + sizeof(BitmapObject) + sizeof(BitmapObject);
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
    BitmapObject::d BitmapObject::dump(bool prevAllocation) {
        Debug::start() + "BO: 0x"  + this + ",0x" + header.prev + ",0x" + header.next + ",0x" + header.firstOffset + ",0x" +
        + header.largestFree + ",0x" + header.pos + ":";
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
            Debug::start() + (dec.val.allocated ? "A" : "F" ) + ",0x" + cum + ",0x" + dec.val.val + ",";
            totalLen += dec.len;
            if(dec.val.allocated) {
                sums.a += dec.val.val;
                if(0 ) {
                    size_t *arr = reinterpret_cast<size_t *>(reinterpret_cast<size_t>(Bitmaps::allocator) + thisOffset);
                    size_t relStart = reinterpret_cast<size_t>(Bitmaps::allocator) + thisOffset;
                    size_t nextJump = 0u;
                    size_t currJump = sizeof(size_t);
                    for (auto i = 0; i < dec.val.val / sizeof(size_t); ++i) {
                        auto initialSize = sizeof(Bitmaps) + sizeof(BitmapObject) + sizeof(BitmapObject);
                        size_t begin = reinterpret_cast<size_t>(Bitmaps::allocator);
                        size_t curr = reinterpret_cast<size_t>(&arr[0]);
                        if (curr < begin + initialSize) {
                            continue;
                        }

                        for (auto tmp = Bitmaps::allocator->first(); ;) {
                            auto stp = reinterpret_cast<ssize_t>(tmp);
                            auto ti = reinterpret_cast<ssize_t>(&arr[i]);
                            auto diff = ti - stp;
                            if (diff >= 0 && diff < sizeof(BitmapObject)) {
                                if (diff == 0) {
                                    relStart += sizeof(BitmapObject);
                                    currJump = sizeof(size_t);
                                }
                                goto done;
                            }
                            tmp = tmp->next();
                            if (tmp == Bitmaps::allocator->first()) {
                                break;
                            }
                        }
                        currJump -= sizeof(size_t);
                        if (currJump == 0) {
                            relStart += nextJump;
                            if (relStart != (size_t) (arr[i] & 0xFFFFFFFF)) {
                                auto stp = nextJump;
                            }
                            nextJump = alignToBits(arr[i] >> 32, 4);
                            currJump = nextJump;
                        }

                        if ((arr[i] & 0xFFFFFFFF) != relStart) {
                            __builtin_trap();
                        }
                        done:
                        continue;
                    }
                }
            } else {
                sums.f += dec.val.val;
                size_t* zeroArray = reinterpret_cast<size_t *>(reinterpret_cast<size_t>(Bitmaps::allocator) + thisOffset);
                for(auto i = 0; i < dec.val.val / sizeof(size_t); ++i) {
                    if(zeroArray[i] != 0ULL) {
                        __builtin_trap();
                    }
                }
            }
            thisOffset += dec.val.val;
            if(!sums.inconsistent && prevAllocation != dec.val.allocated) { sums.inconsistent = true; }
            prevAllocation = dec.val.allocated;
        }
        Debug::start() + "0x" + totalLen + Debug::end;
        if(count() > sizeof(v)) { __builtin_trap(); }
        if(count() != totalLen) { __builtin_trap(); }
        sums.lastAlloc = prevAllocation;
        return sums;
    }

    bool BitmapObject::rebalance() {
        if (count() + REBALANCE_THRESHOLD > sizeof(v) && Bitmaps::allocator->spare != nullptr) {
            auto prevCount = prev()->count();
            auto nextCount = next()->count();
//            if (prevCount > sizeof(v) / 4 || nextCount > sizeof(v) / 4) {
                auto spare = Bitmaps::allocator->getSpare();
                size_t* xx = (size_t*)spare;
                for(int i =0; i < sizeof(*spare)/sizeof(size_t); ++i) {
                    if(xx[i] != ((size_t)(spare) | sizeof(*spare) << 32)) {
                        __builtin_trap();
                    }
                }

                spare->next(next());
                spare->prev(this);
                spare->next()->prev(spare);
                next(spare);
                size_t currOffset = offset();
                DecodedBitmapVal val;
                while(val.pos < count()/ 12) {
                    val = decode(val.pos + val.len);
                    if(val.val.val == 0u) { __builtin_trap(); }
                    currOffset += val.val.val;
                }
                auto split = val.pos + val.len;
                auto sparePos = split;
                while(sparePos < count()) {
                    spare->v.set(sparePos - split, v.get(sparePos));
                    ++sparePos;
                }
                count(split);
                spare->offset(currOffset);
                spare->count(sparePos - split);
                return true;
//            } else {
//                return false;
//                __builtin_trap();
//                // move to next
//            }
//        } else if (count() < REBALANCE_THRESHOLD) {
//            __builtin_trap();
        }
        return false;
    }

    size_t BitmapObject::findBySize(uint32_t const size) {
        rebalance();
        Context con;
        size_t thisOffset = 0u;
        do {
            thisOffset += con.currVal.val.val;
            con.prevVal = con.currVal;
            con.currVal = decode(con.currVal.pos + con.currVal.len);
        } while ((size > con.currVal.val.val || con.currVal.val.allocated) && con.currVal.pos < count());

        if (con.currVal.len == 0u || con.currVal.val.allocated || con.currVal.val.val < size) {
            return 0u;
        }
        if(con.currVal.pos == 0 && this != Bitmaps::allocator->first() ) { __builtin_trap(); }
        if(con.currVal.pos + con.currVal.len == count() && this != Bitmaps::allocator->last()) { __builtin_trap(); }
        con.nextVal = decode(con.currVal.pos + con.currVal.len);
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
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(mergedVal.val) - con.prevVal.len - con.currVal.len;
            auto prevInsertPos = con.currVal.pos - con.prevVal.len;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(newVal, prevInsertPos);
            encLen += encode(mergedVal, prevInsertPos + encLen);
            count(count() + encLen - con.prevVal.len - con.currVal.len);
        } else if (con.currVal.val.val == size) {
            // merge with prev
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(newVal.val) - con.prevVal.len - con.currVal.len - con.nextVal.len;
            auto prevInsertPos = con.currVal.pos - con.prevVal.len;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(newVal, prevInsertPos);
            count(count() + encLen - con.prevVal.len - con.currVal.len - con.nextVal.len);
        }
        return thisOffset + offset();
    }


    bool BitmapObject::findByOffset(size_t const globalOffset, uint32_t const len) {
        if (globalOffset < offset()) {
            return false;
        }
        size_t relativeOffset = globalOffset - offset();
        if(rebalance()) {
            if(globalOffset > next()->offset()) {
                return next()->findByOffset(globalOffset, len);
            }
        }
        Context con;
        auto cumulativeOffset = 0u;
        while (relativeOffset >= cumulativeOffset && con.currVal.pos < count()) {
            con.prevVal = con.currVal;
            con.currVal = decode(con.currVal.pos + con.currVal.len);
            cumulativeOffset += con.currVal.val.val;
        }
        bool borrowedPrev = false;
        if(con.currVal.pos == 0 && this != Bitmaps::allocator->first() ) {
            borrowedPrev = true;
            __builtin_trap();
        }
        bool borrowedNext = false;
        if(con.currVal.pos + con.currVal.len == count() && this != Bitmaps::allocator->last()) { __builtin_trap(); }

        con.nextVal = decode(con.currVal.pos + con.currVal.len);
        if (cumulativeOffset - relativeOffset == con.currVal.val.val && len == con.currVal.val.val) {
            // Merge with prev and next
            BitmapVal mergedVal;
            mergedVal.allocated = false;
            mergedVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(mergedVal.val) - con.prevVal.len - con.currVal.len - con.nextVal.len;
            auto prevInsertPos = con.currVal.pos - con.prevVal.len;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(mergedVal, prevInsertPos);
            count(count() + encLen - con.prevVal.len - con.currVal.len - con.nextVal.len);
            return true;
        } else if (cumulativeOffset - relativeOffset == con.currVal.val.val) {
            BitmapVal prevVal;
            prevVal.allocated = false;
            prevVal.val = len + con.prevVal.val.val;
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.currVal.val.val - len;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(prevVal.val) - con.prevVal.len - con.currVal.len;
            auto prevInsertPos = con.currVal.pos - con.prevVal.len;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(prevVal, prevInsertPos);
            encLen += encode(newVal, prevInsertPos + encLen);
            count(count() + encLen - con.prevVal.len - con.currVal.len);
            return true;
        } else if (cumulativeOffset - relativeOffset == len) {
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.currVal.val.val - len;
            BitmapVal mergedNextVal;
            mergedNextVal.allocated = false;
            mergedNextVal.val = con.nextVal.val.val + len;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(mergedNextVal.val) - con.currVal.len - con.nextVal.len;
            auto prevInsertPos = con.currVal.pos;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(newVal, prevInsertPos);
            encLen += encode(mergedNextVal, prevInsertPos + encLen);
            count(count() + encLen - con.currVal.len - con.nextVal.len);
            return true;
        } else {
            BitmapVal prevVal;
            prevVal.allocated = true;
            prevVal.val = con.currVal.val.val + relativeOffset - cumulativeOffset;
            BitmapVal newVal;
            newVal.allocated = false;
            newVal.val = len;
            BitmapVal nextVal;
            nextVal.allocated = true;
            nextVal.val = cumulativeOffset - relativeOffset - len;
            ssize_t shiftLen = v.getLen(newVal.val) + v.getLen(prevVal.val) + v.getLen(nextVal.val) - con.currVal.len;
            auto prevInsertPos = con.currVal.pos;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(prevVal, prevInsertPos);
            encLen += encode(newVal, prevInsertPos + encLen);
            encLen += encode(nextVal, prevInsertPos + encLen);
            count(count() + encLen - con.currVal.len);
            return true;
        }
    }
}