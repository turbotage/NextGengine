#pragma once

#include <assert.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include <vulkan\vulkan.h>

#ifdef __linux__
#define ALIGN(s) __attribute__((aligned(s)))
#elif _WIN32
#define ALIGN(s) __declspec(align(s))
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

//inherit to stop class from being copyable 
struct NotCopyable {
public:
	NotCopyable() = default;
	NotCopyable(NotCopyable const &NotCopyable) = delete;
	NotCopyable &operator =(NotCopyable const &NotCopyable) = delete;
};

