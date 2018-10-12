#pragma once

#include "../../def.h"
#include "../vulkan_device.h"
#include <vector>

namespace ng {
	namespace vulkan {

		class VulkanPipeline
		{
		private:

		protected:

			std::string m_Qualifier;

			VkPipeline m_Pipeline;

			VkRenderPass m_RenderPass;

			VkPipelineLayout m_PipelineLayout;

			std::vector<VkShaderModule> m_ShaderModules;

		protected:
			
			VulkanPipeline();
			~VulkanPipeline();


			VkShaderModule createShaderModule(VulkanDevice* device, const std::vector<char>& code);

		};

	}
}

