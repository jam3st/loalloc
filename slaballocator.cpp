#include "slaballocator.hpp"

namespace Gx {

    void *initBrk();

    size_t extendBrk(void *const alloc, size_t const len);

    Bitmaps *Bitmaps::allocator = nullptr;

    void Bitmaps::init() {
        allocator = reinterpret_cast<Bitmaps *>(initBrk());
        constexpr size_t initialUsed = sizeof(Bitmaps) + sizeof(BitmapObject);
        constexpr size_t initialAlloc = roundUpNearestMultiple(initialUsed, minPageFrameSize);
        if(extendBrk(allocator, initialAlloc) != initialAlloc) { __builtin_trap(); }
        allocator->initMaps(initialUsed, initialAlloc - initialUsed);
        allocator->dump();
    }

    void Bitmaps::extend(size_t const size) {
        auto extensionSize = roundUpNearestMultiple(size, minPageFrameSize);
        if(allocLength + extensionSize == allocLength) { __builtin_trap(); }
        allocLength += extensionSize;
        if (extendBrk(allocator, allocLength) != allocLength) { __builtin_trap(); }
        getFirst()->prev()->append(extensionSize, false);
    }

    void Bitmaps::contract(size_t const size) {
        auto contractionSize = roundDownNearestMultiple(size, minPageFrameSize);
        allocLength -= contractionSize;
        if (extendBrk(allocator, allocLength) != allocLength) { __builtin_trap(); }
        if(size < contractionSize) { __builtin_trap(); }
        if(size != contractionSize) {
            getFirst()->prev()->append(size - contractionSize, false);
        }

    }

    size_t BitmapObject::findBySize(uint32_t const size) {
        Context con;
        con.currVal.val.allocated = true;
        size_t thisOffset = 0u;
        while ((size < con.currVal.val.val || con.currVal.val.allocated) && con.pos < count()) {
            thisOffset += con.currVal.val.val;
            con.prevVal = con.currVal;
            con.currVal = decode(con.pos);
            con.pos += con.currVal.len;
        }
        if (!con.currVal.val.allocated) {
            con.found = true;
        }
        if (!con.found) {
            return 0u;
        }

        con.nextVal = decode(con.pos);
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
            v.shift(shiftLen, con.pos - con.prevVal.len - con.currVal.len, count());
            auto prevInsertPos = con.pos - con.prevVal.len - con.currVal.len;
            size_t encLen = encode(newVal, prevInsertPos);
            encLen += encode(mergedVal, prevInsertPos + encLen);
            count(count() + encLen - con.prevVal.len - con.currVal.len);
            return thisOffset;
        } else if (con.currVal.val.val == size) {
            // merge with prev
            BitmapVal newVal;
            newVal.allocated = true;
            newVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(newVal.val) - con.prevVal.len - con.currVal.len;
            v.shift(shiftLen, con.pos - con.prevVal.len - con.currVal.len, count());
            auto prevInsertPos = con.pos - con.prevVal.len - con.currVal.len;
            size_t encLen = encode(newVal, prevInsertPos);
            count(count() + encLen - con.prevVal.len - con.currVal.len - con.nextVal.len);
            return thisOffset;
        }
        return 0u;
    }


    bool BitmapObject::findByOffset(size_t const globalOffset, uint32_t const len) {
        if (globalOffset < offset()) {
            return false;
        }
        Context con;
        con.found = true;
        auto const relativeOffset = globalOffset - offset();
        auto cumulativeOffset = 0u;
        while (relativeOffset >= cumulativeOffset && con.pos < count()) {
            con.prevVal = con.currVal;
            con.currVal = decode(con.pos);
            cumulativeOffset += con.currVal.val.val;
            con.pos += con.currVal.len;
        }
        con.nextVal = decode(con.pos);
        ssize_t diff = cumulativeOffset - relativeOffset;
        if (cumulativeOffset - relativeOffset == con.currVal.val.val && len == con.currVal.val.val) {
            // Merge with prev and next
            BitmapVal mergedVal;
            mergedVal.allocated = false;
            mergedVal.val = con.prevVal.val.val + con.currVal.val.val + con.nextVal.val.val;
            ssize_t shiftLen = v.getLen(mergedVal.val) - con.prevVal.len - con.currVal.len - con.nextVal.len;
            v.shift(shiftLen, con.pos - con.prevVal.len - con.currVal.len, count());
            auto prevInsertPos = con.pos - con.prevVal.len - con.currVal.len;
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
            v.shift(shiftLen, con.pos - con.prevVal.len - con.currVal.len, count());
            auto prevInsertPos = con.pos - con.prevVal.len - con.currVal.len;
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
            auto prevInsertPos = con.pos - con.currVal.len;
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
            auto prevInsertPos = con.pos - con.currVal.len;
            v.shift(shiftLen, prevInsertPos, count());
            size_t encLen = encode(prevVal, prevInsertPos);
            encLen += encode(newVal, prevInsertPos + encLen);
            encLen += encode(nextVal, prevInsertPos + encLen);
            count(count() + encLen - con.currVal.len);
            return true;
        }
        return false;
    }
}