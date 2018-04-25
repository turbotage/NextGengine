#include "vulkan_model.h"
#include "../../Math/hash_functions.h"

void ng::graphics::VulkanModel::init(VulkanBase* vulkanBase, VkDescriptorSetLayout * descriptorSetLayout)
{
	//m_VulkanBase = vulkanBase;
	//m_DescSetLayout = descriptorSetLayout;
}

void ng::graphics::VulkanModel::loadModel(const char * filename)
{
	/*
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = m_VulkanBase->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = m_DescSetLayout;

	m_DescriptorSet = m_VulkanBase->graphicsUnit.device.allocateDescriptorSets(allocInfo)[0];
	*/

}

ng::graphics::VulkanModel ng::graphics::VulkanModel::operator=(const VulkanModel & model)
{
	VulkanModel ret;
	ret.VIBO = model.VIBO;
	


}

std::size_t ng::graphics::VulkanModel::hash(VulkanModel const & model)
{
	std::size_t ret = ng::memory::VulkanBuffer::hash(model.VIBO);
	ng::math::hashCombine(ret, ng::graphics::VulkanTexture::hash(model.textureArray));
	return ret;
}
