#include "vulkan_texture.h"

void ng::graphics::VulkanTexture::updateDescriptor()
{
	descriptor.sampler = sampler;
	descriptor.imageView = view;
	descriptor.imageLayout = imageLayout;
}

void ng::graphics::VulkanTexture::destroy()
{
	vkDestroyImageView(vulkanDevice->logicalDevice, view, nullptr);
	vkDestroyImage(vulkanDevice->createLogicalDevice, image, nullptr);
	if (sampler) {
		vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
	}

}
