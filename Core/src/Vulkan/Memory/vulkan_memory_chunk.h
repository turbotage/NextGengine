#pragma once

#include "../../def.h"
#include "../../debug.h"
#include "../vulkan_device.h"

namespace ng {
	namespace vulkan {

		struct Block {
			VkDeviceSize offset;
			VkDeviceSize size;

			static bool equalOffset(const Block& left, const Block& right) {
				return left.offset == right.offset;
			}

			static bool equalOffsetPlusSize(const Block& left, const Block& right) {
				return (left.offset + left.size) == (right.offset + right.size);
			}

			static bool equalOffsetPlusSizeAsOffset(const Block& left, const VkDeviceSize& right) {
				return (left.offset + left.size) == right;
			}

		};

	}
}