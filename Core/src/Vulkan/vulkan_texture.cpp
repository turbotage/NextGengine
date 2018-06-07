#include "vulkan_texture.h"
#include "../Memory/vulkan_buffer.h"

#include <gli\gli.hpp>


void ng::vulkan::VulkanTexture::updateDescriptor()
{
	descriptor.sampler = sampler;
	descriptor.imageView = view;
	descriptor.imageLayout = imageLayout;
}

void ng::vulkan::VulkanTexture::destroy(VulkanDevice* vulkanDevice)
{
	vkDestroyImageView(vulkanDevice->logicalDevice, view, nullptr);
	vkDestroyImage(vulkanDevice->logicalDevice, image, nullptr);
	if (sampler) {
		vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
	}

}

std::size_t ng::vulkan::VulkanTexture::hash(VulkanTexture const& texture)
{
	return ng::vulkan::VulkanBuffer::hash(texture.textureBuffer);
}

void ng::vulkan::VulkanTexture2D::loadFromFile(std::string filename, VulkanDevice * vulkanDevice, VkFormat format, VkQueue copyQueue, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout, bool forceLinear)
{
	using namespace gli::gli;

	texture2d texture2D(load(filename.c_str()));

	assert(!texture2D.empty());

	width = static_cast<uint32>(texture2D.extent().x);
	height = static_cast<uint32>(texture2D.extent().y);
	mipLevels = static_cast<uint32>(texture2D.levels());

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vulkanDevice->physicalDevice, format, &formatProperties);

	VkBool32 useStaging = !forceLinear;

	VkMemoryAllocateInfo memAllocInfo;
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkMemoryRequirements memReqs;
	
	VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

}
