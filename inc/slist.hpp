#pragma once

#include "types.hpp"

namespace Gx {


	template <typename T>
	class Object { ;
	public:
		Object(void* const objPtr) :  objPtr(objPtr), flags(Flags::Empty), refCount(0) {

		}
		Object() = delete;

		~Object() {
		};

		enum class Flags : uint16_t {
			Empty = 1u << 0,
			Moving = 1u << 1,
			Whatever = 1u << 15
		};

		void IncRef() {
			objPtr = nullptr;

		}
	protected:
		void DefRef() {
			objPtr = nullptr;
		}
	protected:
	private:
		void* const objPtr;
		Flags flags;
		uint16_t refCount;
	};

	template <typename T>
	class DoubleList {
	public:
		DoubleList(DoubleList<T>* const objPtr) : obj(objPtr),  prev(this), next(this)  {
		}

		DoubleList(DoubleList<T> const&) = delete;

		~DoubleList() {
		}

		DoubleList<T>* Insert(DoubleList<T> const& what) {

		}

		DoubleList<T>* Remove(DoubleList<T> const& what) {

		}

		DoubleList<T>* Next() const {
			return next;
		}

		DoubleList<T>& operator=(DoubleList<T> const&) = delete;
	private:
		DoubleList<T>* next;
		DoubleList<T>* prev;
		T obj;
	};
}