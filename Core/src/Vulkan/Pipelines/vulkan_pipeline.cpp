#include "vulkan_pipeline.h"
#include <fstream>
#include <string>
#include <iostream>

VkShaderModule ng::vulkan::VulkanPipeline::createShaderModule(VulkanDevice* device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
