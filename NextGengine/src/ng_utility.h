#pragma once

// Class helpers
namespace ng {

	/*
	Shall denote that the class can only be instanced via a smart_ptr
	Example:

	class A : public MakeConstructed {
	public:
		//this function is used to instanciate an object
		static std::unique_ptr<A> make() {
			return std::make_unique<A>();
		}

		~A() = default;
	private: (or protected)
		A() = default;
		A(const A&) = delete;
		A& operator=(const A&) = delete;
	};
	*/
	/*
	template<class T>
	class MakeConstructed : public std::enable_shared_from_this<T> {};
	*/
	class MakeConstructed {};





	/*
	Shall denote that the class can only be created via a allocator, and the allocator should
	only return a smart pointer to the created instance
	Example:

	class A : public AllocatorConstructed {
	public:

		~A() = default;
	private: (or protected)
		A() = default;
		A(const A&) = delete;
		A& operator=(const A&) = delete;

		friend class AAllocator;
	};

	class AAllocator {
	public:
		std::unique_ptr<A> allocateA() {
			return std::unique_ptr<A>(new A());
		}
	}
	*/
	class AllocatorConstructed {};





	/* To measure structure size without compilation of running
		Example:
		int x;
		Sizer<sizeof(x)> foo; (hover over foo and intelisense will show the size)
	*/
	template <size_t S> class Sizer {};






	template<class T>
	class EnableSharedThis : public std::enable_shared_from_this<T> {};


}

// Pointers
namespace ng {

	template<typename T>
	class raw_ptr {
	public:

		raw_ptr() = default;

		raw_ptr(const raw_ptr& other) { ptr = other.ptr; }
		raw_ptr(T* other) { ptr = other; }

		T& operator*() { return *ptr; }
		T* operator->() const { return ptr; }

		friend bool operator==(const raw_ptr& l, const raw_ptr& r) { return (l.ptr == r.ptr); }
		friend bool operator==(const T* l, const raw_ptr& r) { return (l == r.ptr); }
		friend bool operator==(const raw_ptr& l, const T* r) { return (l.ptr == r); }
		friend bool operator!=(const raw_ptr& l, const raw_ptr& r) { return (l.ptr != r.ptr); }
		friend bool operator!=(const T* l, const raw_ptr& r) { return (l == r.ptr); }
		friend bool operator!=(const raw_ptr& l, T* r) { return (l.ptr == r); }

	private:
		T* ptr;
	};

	


	template<class T>
	class count_ptr {

	};

}