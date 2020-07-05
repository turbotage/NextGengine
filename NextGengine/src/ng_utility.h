#pragma once

#include "pch.h"

// Helver functions
namespace ng {

	typedef uint8_t uint8;

	struct Vertex2D { glm::vec2 vertex; };
	struct Vertex2D_3C { glm::vec2 vertex; glm::vec3 color; };
	struct Vertex3D { glm::vec3 vertex; };


	// Returns a string to the shader directory
	std::string getShaderDirectoryPath(std::string projectName = "NextGengine");
	// Returns a string to the assets directory
	std::string getAssetsDirectoryPath(std::string projectName = "NextGengine");
	// Returns a list o fthe directories and the shaders in the directories
	std::string listShaders(std::string projectName = "NextGengine");
	// Return the string of the extension, (the stuff after the dot)
	std::string getFileExtension(std::string filename);
	// Load a file
	std::vector<uint8> loadFile(const std::string& filename);
	// Write to file
	void writeFile(std::vector<uint8> bytes, const std::string& filename);
	// Load NG file, (virtual filing) // Not yet implemented
	std::vector<uint8> loadNGFile(const std::string& filename);

	// Get floats from bytes
	std::vector<float> getFloatsFromBytes(std::vector<uint8> bytes);
	// Get Vertex2D from bytes
	std::vector<Vertex2D> getVertex2DsFromBytes(std::vector<uint8> bytes);
	// Get Vertex2D_3C from bytes
	std::vector<Vertex2D_3C> getVertex2D_3CsFromBytes(std::vector<uint8> bytes);
	// Get Vertex3D from bytes
	std::vector<Vertex3D> getVertex3DsFromBytes(std::vector<uint8> bytes);
	// Get bytes from floats
	std::vector<uint8> getBytesFromFloats(std::vector<float> floats);
	// Get bytes from Vertex2D
	std::vector<uint8> getBytesFromVertex2Ds(std::vector<Vertex2D> vertices);
	// Get bytes from Vertex2D_3C
	std::vector<uint8> getBytesFromVertex2D_3Cs(std::vector<Vertex2D_3C> vertices);
	// Get bytes from Vertex3D
	std::vector<uint8> getBytesFromVertex3Ds(std::vector<Vertex3D> vertices);
}

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

	template<typename T> using raw_ptr = T*;

	/*
	template<typename T>
	class raw_ptr {
	public:

		raw_ptr() = default;

		raw_ptr(const raw_ptr& other) { ptr = other.ptr; }
		raw_ptr(T* other) { ptr = other; }

		T* get() { return ptr; }

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
	*/

	

	/*
	template<class T>
	class count_ptr {

	};
	*/

}