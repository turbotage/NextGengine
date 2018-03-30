#pragma once

#include "../def.h"

#define VK_FLAGS_NONE 0

#define VULKAN_CHECK_RESULT(f)																						\
{																													\
	VkResult res = (f);																								\
	if (res != VK_SUCCESS) {																						\
		std::cout << "VkResukt is \"" << f << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl;			\
		assert(res == VK_SUCCESS);																					\
	}																												\
}

namespace ng {
	namespace graphics {

		class VulkanTools
		{
		public:
			VulkanTools();
			~VulkanTools();



		};

	}
}


