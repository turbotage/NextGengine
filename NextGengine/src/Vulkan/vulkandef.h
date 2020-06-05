#pragma once
#include "../def.h"

//#include "../NextGengine.h"

#include <vulkan/vulkan.hpp>

#define NEXTGENGINE_SPIRV_SUPPORT

#ifdef NEXTGENGINE_SPIRV_SUPPORT
#include <spirv-headers/spirv.hpp11>
#endif