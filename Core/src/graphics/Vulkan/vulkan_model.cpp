#include "vulkan_model.h"

void ng::graphics::VulkanModel::init(VulkanBase* vulkanBase, vk::DescriptorSetLayout * descriptorSetLayout)
{
	m_VulkanBase = vulkanBase;
	m_DescSetLayout = descriptorSetLayout;
}

void ng::graphics::VulkanModel::loadModel(const char * filename)
{
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = m_VulkanBase->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = m_DescSetLayout;

	m_DescriptorSet = m_VulkanBase->graphicsUnit.device.allocateDescriptorSets(allocInfo)[0];



}
