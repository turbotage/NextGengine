#pragma once

#include "../../def.h"

#include <vector>

namespace ng {
	namespace graphics {
		class DeferredRenderer
		{
		public:
			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;



			DeferredRenderer();
			~DeferredRenderer();
		};
	}
}


