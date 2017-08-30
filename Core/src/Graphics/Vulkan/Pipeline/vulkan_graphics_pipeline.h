#pragma once
#include "vulkan_pipeline.h"
#include "../../window.h"


namespace ng {
	namespace graphics {

		enum NgShaderType {
			VERTEX_SHADER_BIT = 5,
			TESSELLATION_SHADER_BIT = 10,
			FRAGMENT_SHADER_BIT = 20
		};

		class VulkanGraphicsPipeline : VulkanPipeline
		{
			VkDevice* m_Device;

			Window* m_Window;
		public:

			VkRenderPass renderPass;
			VkPipelineLayout pipelineLayout;

			VkShaderModule vertShaderModule = VK_NULL_HANDLE;
			VkShaderModule tessellShaderModule = VK_NULL_HANDLE;
			VkShaderModule fragShaderModule = VK_NULL_HANDLE;

			void createRenderPass();
			void freeRenderPass();

			void createGraphicsPipeline(Window* window, std::vector<std::pair<std::string, NgShaderType>>* shaderPaths);
			void freeGraphicsPipeline();

		};
	}
}

