#pragma once

#include "../../def.h"
#include <vector>

namespace ng {
	namespace graphics {

		class VulkanPipeline
		{
		protected:

			static std::vector<char> readFile(const std::string& filename);

			VkShaderModule createShaderModule(VkDevice* device, const std::vector<char>& code);

		};

	}
}

