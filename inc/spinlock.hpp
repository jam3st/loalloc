#pragma once

#include "types.hpp"
#include "atomics.hpp"
namespace Gx {
	class SpinIncrementLock16 final {
	public:
		bool acquiredWriteLock() {
			LockState currState{.state16 = Atomic::Load(lock.state16)};
			LockState newState{currState};
			if (newState.state.numReaders == 0 && !newState.state.writing) {
				newState.state.writing = true;
				return Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16);
			}
			return false;
		}

		bool acquiredReadLock() {
			LockState currState{.state16 = Atomic::Load(lock.state16)};
			LockState newState{currState};
			if (!newState.state.writing) {
				++newState.state.numReaders;
				return Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16);
			}
			return false;
		}

		void lockWriting() {
			while (!acquiredWriteLock()) {
			}
		}

		void lockReading() {
			while (!acquiredReadLock()) {
			}
		}

		void unLockRead() {
			for (; ;) {
				LockState currState{.state16 = Atomic::Load(lock.state16)};
				LockState newState{currState};
				--newState.state.numReaders;
				if (Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16)) {
					break;
				}
			}
		}

		void unlockWriting() {
			for (; ;) {
				LockState currState{.state16 = Atomic::Load(lock.state16)};
				LockState newState{currState};
				newState.state.writing = false;
				if (Atomic::CompareAndSet(lock.state16, currState.state16, newState.state16)) {
					break;
				}
			}
		}

	private:
		typedef union {
			struct {
				uint16_t numReaders : 15;
				bool writing : 1;
			} state;
			uint16_t state16;
		} LockState;
		LockState lock;
	};
}