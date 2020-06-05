#pragma once

#include <stdint.h>
#include <inttypes.h>


#define NOMINMAX

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8 byte;
typedef uint32_t uint;

#include <memory>
#include <list>
#include <vector>
#include <map>

namespace ng {
	
	/*
	Shall denote that the class can only be instanced via a smart_ptr, also enables shared_ptr from this
	Example:

	class A : public MakeConstructed<A> {
	public:
		//this function is used to instanciate an object
		static std::unique_ptr<A> make() { 
			return std::make_unique<A>();
		}

		~A() = default;
	private:
		A() = default;
		A(const A&) = delete;
		A& operator=(const A&) = delete;
	};
	*/
	template<class T>
	class MakeConstructed : public std::enable_shared_from_this<T> {};


	/* To measure structure size without compilation of running
		Example:
		int x;
		Sizer<sizeof(x)> foo; (hover over foo and intelisense will show the size)
	*/
	template <size_t S> class Sizer {};

}

namespace std {

	template<typename T>
	class raw_ptr {
	public:

		raw_ptr() = default;

		raw_ptr(const raw_ptr& other) { this->ptr = other->ptr; }
		raw_ptr(T* other) { this->ptr = other; }

		T* operator=(const raw_ptr& other);

		T& operator*() { return *ptr; }

		T* operator->() const { return ptr; }

	private:
		T* ptr;
	};

	template<class T>
	class count_ptr {

	};
}