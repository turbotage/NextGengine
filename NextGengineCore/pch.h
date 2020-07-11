#pragma once

// VULKAN
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.hpp>

#define NEXTGENGINE_SPIRV_SUPPORT

#ifdef NEXTGENGINE_SPIRV_SUPPORT
#include <spirv-headers/spirv.hpp11>
#endif

// GLFW
#include <GLFW/glfw3.h>


// GLM
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// TINYGLTF
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NOEXCEPTION
#include <tiny_gltf.h>


// STL
#include <filesystem>

#include <fstream>
#include <iostream>

#include <map>
#include <unordered_map>
#include <queue>
#include <vector>
#include <list>
#include <string>
#include <array>

#include <functional>
#define NOMINMAX
#include <algorithm>
#include <memory>

#include <mutex>
#include <thread>


#define NG_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define NG_ENGINE_NAME "NextGengine"
#define NG_ENGINE_VERSION NG_MAKE_VERSION(1,0,0)


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

