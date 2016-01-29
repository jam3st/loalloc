#pragma once

#include "types.hpp"
#include "alloctree.hpp"
#include "sysconfig.hpp"

namespace Gx {

	inline void assert(bool const cond) {
		if(!cond) {
			__builtin_trap();
		}
	}
	class SpinIncrementLock16 final {
	public:
		bool acquiredWriteLock() {
			LockState currState{ .state16 = Atomic::Load(lock.state16) };
			LockState newState{ currState };
			if(newState.state.numReaders == 0 && !newState.state.writing) {
				newState.state.writing = true;
				return Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16);
			}
			return false;
		}

		bool acquiredReadLock() {
			LockState currState{ .state16 = Atomic::Load(lock.state16) };
			LockState newState{ currState };
			if(!newState.state.writing) {
				++newState.state.numReaders;
				return Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16);
			}
			return false;
		}

		void lockWriting() {
			while(!acquiredWriteLock()){
			}
		}

		void lockReading() {
			while(!acquiredReadLock()){
			}
		}

		void unLockRead() {
			for(;;) {
				LockState currState{ .state16 = Atomic::Load(lock.state16) };
				LockState newState{ currState };
				assert(!newState.state.writing);
				--newState.state.numReaders;
				if(Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16)) {
					break;
				}
			}
		}

		void unlockWriting() {
			for(;;) {
				LockState currState{ .state16 = Atomic::Load(lock.state16) };
				LockState newState{ currState };
				assert(newState.state.numReaders == 0);
				assert(newState.state.writing);
				newState.state.writing = false;
				if(Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16)) {
					break;
				}
			}
		}

	private:
		typedef union {
			struct {
				uint16_t numReaders : 14;
				bool writing : 1;
			} state;
			uint16_t state16;
		} LockState;
		LockState lock;
	};

	class MemItem final {
	public:
		MemItem* prev;
		MemItem* next;
		ssize_t len;
		uint16_t refCount;
		SpinIncrementLock16 lock;
	};

	struct Cache {
		void* todo;
	};

	struct BinItem final {

	};

	struct Bin final {
		BinItem* bins;
		uint32_t maxSize;
		uint32_t numberOfItems;
		SpinIncrementLock16 lock;
	};


}
