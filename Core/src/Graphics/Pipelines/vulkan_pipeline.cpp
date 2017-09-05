#include "vulkan_pipeline.h"
#include <fstream>
#include <string>
#include <iostream>

std::vector<char> ng::graphics::VulkanPipeline::readFile(const std::string & filename)
{
	std::string filename2 = "CompiledShaders/" + filename;
	std::ifstream file(filename2, std::ios::ate | std::ios::binary);
	std::cout << filename2 << std::endl;
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

VkShaderModule ng::graphics::VulkanPipeline::createShaderModule(VkDevice* device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
