#pragma once
#include "vulkan_pipeline.h"
#include "../window.h"


namespace ng {
	namespace graphics {

		enum NgShaderType {
			VERTEX_SHADER_BIT = 5,
			TESSELLATION_SHADER_BIT = 10,
			FRAGMENT_SHADER_BIT = 20
		};

		class VulkanGraphicsPipeline : public VulkanPipeline
		{
		private:
			VkDevice * m_Device;
			Window * m_Window;
		public:

			VkPipeline graphicsPipeline;
			VkRenderPass renderPass;
			VkPipelineLayout pipelineLayout;

			VkShaderModule vertShaderModule = VK_NULL_HANDLE;
			VkShaderModule tessellShaderModule = VK_NULL_HANDLE;
			VkShaderModule fragShaderModule = VK_NULL_HANDLE;

			void createRenderPass(VkDevice* device, Window* window);
			void freeRenderPass();

			void createGraphicsPipeline(std::vector<std::pair<std::string, NgShaderType>>* shaderPaths);
			void freeGraphicsPipeline();

		};
	}
}

