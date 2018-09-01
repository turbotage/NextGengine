#pragma once

#include "../../def.h"
#include <vector>

namespace ng {
	namespace vulkan {

		class VulkanPipeline
		{
		private:

			std::string qualifier;

			VkPipeline pipeline;

			VkRenderPass renderPass;

			VkPipelineLayout pipelineLayout;

			std::vector<VkShaderModule> shaderModules;

		protected:

			VulkanPipeline();
			~VulkanPipeline();


			VkShaderModule createShaderModule(VkDevice* device, const std::vector<char>& code);

		};

	}
}

