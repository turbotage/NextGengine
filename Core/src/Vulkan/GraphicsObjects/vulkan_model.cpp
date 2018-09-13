#include "vulkan_model.h"
#include "../../Math/hash_functions.h"

void ng::vulkan::VulkanModel::init(VulkanBase* vulkanBase, VkDescriptorSetLayout * descriptorSetLayout)
{
	//VulkanBase = vulkanBase;
	//DescSetLayout = descriptorSetLayout;
}

void ng::vulkan::VulkanModel::loadModel(const char * filename)
{
	/*
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = VulkanBase->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = DescSetLayout;

	DescriptorSet = VulkanBase->graphicsUnit.device.allocateDescriptorSets(allocInfo)[0];
	*/

}

ng::vulkan::VulkanModel ng::vulkan::VulkanModel::operator=(const VulkanModel & model)
{
	VulkanModel ret;
	ret.VIBO = model.VIBO;

}

std::size_t ng::vulkan::VulkanModel::hash(VulkanModel const & model)
{
	std::size_t ret = VulkanBuffer::hash(model.VIBO);
	ngm::hashCombine(ret, ng::vulkan::VulkanTexture::hash(model.textureArray));
	return ret;
}
