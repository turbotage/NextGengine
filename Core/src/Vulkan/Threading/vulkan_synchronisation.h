#pragma once

#include "../../def.h"

/**
	g_MemoryLock :
		All work that is using any handles to VulkanBuffer memory should lock around this mutex, this is to ensure that
		no work is recorded using handles pointing to old memory locations.







**/

namespace ng {
	namespace vulkan {

		namespace globals {
			//std::mutex g_MemoryLock;


		}

	}
}