#pragma once

#include <assert.h>
#include <math.h>
#include <complex>
#include <valarray>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <mutex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include <vulkan\vulkan.h>

#ifdef __linux__
#define ALIGN(s) __attribute__((aligned(s)))
#elif defined(_WIN32)
#define ALIGN(s) __declspec(align(s))
#include <fcntl.h>
#include <io.h>
#else 
#define ALIGN(s) __attribute__((aligned(s)))
#endif

#define NG_PI 3.14159265358979323846264338327950288419716939937510582097

#define TO_RADIANS(degrees) degrees * NG_PI / 180

#define TO_DEGREES(radians) radians * 180 / NG_PI

#include <stdint.h>
#include <inttypes.h>

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

typedef uint8 VkMemoryAlignment;

#include <array>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <forward_list>

/*
namespace ng {
	namespace stl {
		template<typename T, typename R>
		searchMultimapByValue(std::multimap<T, R>::interator searchStart, std::multimap<T, R>::interator searchEnd, R searchValue) {
			using const_ref = std::multimap<T, R>::const_reference;
			std::find_if()
		}
	}
}
*/

//inherit to stop class from being copyable 
struct NotCopyable {
public:
	NotCopyable() = default;
	NotCopyable(NotCopyable const &NotCopyable) = delete;
	NotCopyable &operator =(NotCopyable const &NotCopyable) = delete;
};

namespace tools {

	std::vector<char> readFile(const std::string & filename)
	{
		std::string filename2 = "CompiledShaders/" + filename;
		std::ifstream file(filename2, std::ios::ate | std::ios::binary);
		std::cout << filename2 << std::endl;
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}



}