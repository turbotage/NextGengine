#pragma once

#include <stdint.h>
#include <inttypes.h>


#define NOMINMAX

#define NG_MAKE_VERSION(major, minor, patch) \
    ((((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))

#define NG_ENGINE_NAME "NextGengine"
#define NG_ENGINE_VERSION NG_MAKE_VERSION(1,0,0)

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

#include "ng_utility.h"