#include "vulkan_descriptor.h"

#include "vulkan_device.h"

// <====================== DESCRIPTOR SET LAYOUT MAKER =============================>

void ngv::VulkanDescriptorSetLayoutMaker::buffer(uint32 binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
{
	m_Bindings.emplace_back(binding, descriptorType, descriptorCount, stageFlags, nullptr);
}

void ngv::VulkanDescriptorSetLayoutMaker::image(uint32 binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
{
	m_Bindings.emplace_back(binding, descriptorType, descriptorCount, stageFlags, nullptr);
}

void ngv::VulkanDescriptorSetLayoutMaker::samplers(uint32 binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, const std::vector<vk::Sampler> immutableSamplers)
{
	m_Samplers.push_back(immutableSamplers);
	auto pImmutableSamplers = m_Samplers.back().data();
	m_Bindings.emplace_back(binding, descriptorType, (uint32)immutableSamplers.size(), stageFlags, pImmutableSamplers);
}

void ngv::VulkanDescriptorSetLayoutMaker::bufferView(uint32 binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
{
	m_Bindings.emplace_back(binding, descriptorType, descriptorCount, stageFlags, nullptr);
}

vk::UniqueDescriptorSetLayout ngv::VulkanDescriptorSetLayoutMaker::createUnique(VulkanDevice& device) const
{
	vk::DescriptorSetLayoutCreateInfo ci{};
	ci.bindingCount = (uint32)m_Bindings.size();
	ci.pBindings = m_Bindings.data();
	return device.device().createDescriptorSetLayoutUnique(ci);
}







// <======================== DESCRIPTOR SET MAKER =============================>

void ngv::VulkanDescriptorSetMaker::layout(vk::DescriptorSetLayout layout)
{
	m_Layouts.push_back(layout);
}

std::vector<vk::DescriptorSet> ngv::VulkanDescriptorSetMaker::create(VulkanDevice& device, vk::DescriptorPool descriptorPool) const
{
	vk::DescriptorSetAllocateInfo dsai{};
	dsai.descriptorPool = descriptorPool;
	dsai.descriptorSetCount = (uint32)m_Layouts.size();
	dsai.pSetLayouts = m_Layouts.data();
	return device.device().allocateDescriptorSets(dsai);
}

std::vector<vk::UniqueDescriptorSet> ngv::VulkanDescriptorSetMaker::createUnique(VulkanDevice& device, vk::DescriptorPool descriptorPool) const
{
	vk::DescriptorSetAllocateInfo dsai{};
	dsai.descriptorPool = descriptorPool;
	dsai.descriptorSetCount = (uint32)m_Layouts.size();
	dsai.pSetLayouts = m_Layouts.data();
	return device.device().allocateDescriptorSetsUnique(dsai);
}









// <========================== DESCRIPTOR SET UPDATER ===============================>

ngv::VulkanDescriptorSetUpdater::VulkanDescriptorSetUpdater(int maxBuffers, int maxImages, int maxBufferViews = 0)
{
	m_BufferInfos.resize(maxBuffers);
	m_ImageInfos.resize(maxImages);
	m_BufferViews.resize(maxBufferViews);
}

void ngv::VulkanDescriptorSetUpdater::beginDescriptorSet(vk::DescriptorSet dstSet)
{
	m_DescSet = dstSet;
}

void ngv::VulkanDescriptorSetUpdater::beginImages(uint32 dstBinding, uint32 dstArrayElement, vk::DescriptorType descriptorType)
{
	vk::WriteDescriptorSet wdesc{};
	wdesc.dstSet = m_DescSet;
	wdesc.dstBinding = dstBinding;
	wdesc.dstArrayElement = dstArrayElement;
	wdesc.descriptorCount = 0;
	wdesc.descriptorType = descriptorType;
	wdesc.pImageInfo = m_ImageInfos.data() + m_NumImages;
	m_DescriptorWrites.push_back(wdesc);

}

void ngv::VulkanDescriptorSetUpdater::image(vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout imageLayout)
{
	if (!m_DescriptorWrites.empty() && m_NumImages != m_ImageInfos.size() && m_DescriptorWrites.back().pImageInfo) {
		m_DescriptorWrites.back().descriptorCount++;
		m_ImageInfos[m_NumImages++] = vk::DescriptorImageInfo{ sampler, imageView, imageLayout };
	}
	else {
		m_Ok = false;
	}
}

void ngv::VulkanDescriptorSetUpdater::beginBuffers(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType)
{
	vk::WriteDescriptorSet wdesc{};
	wdesc.dstSet = m_DescSet;
	wdesc.dstBinding = dstBinding;
	wdesc.dstArrayElement = dstArrayElement;
	wdesc.descriptorCount = 0;
	wdesc.descriptorType = descriptorType;
	wdesc.pBufferInfo = m_BufferInfos.data() + m_NumBuffers; // scary!
	m_DescriptorWrites.push_back(wdesc);
}

void ngv::VulkanDescriptorSetUpdater::buffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
{
	if (!m_DescriptorWrites.empty() && m_NumBuffers != m_BufferInfos.size() && m_DescriptorWrites.back().pBufferInfo) {
		m_DescriptorWrites.back().descriptorCount++;
		m_BufferInfos[m_NumBuffers++] = vk::DescriptorBufferInfo{ buffer, offset, range };
	}
	else {
		m_Ok = false;
	}
}

void ngv::VulkanDescriptorSetUpdater::beginBufferViews(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType)
{
	vk::WriteDescriptorSet wdesc{};
	wdesc.dstSet = m_DescSet;
	wdesc.dstBinding = dstBinding;
	wdesc.dstArrayElement = dstArrayElement;
	wdesc.descriptorCount = 0;
	wdesc.descriptorType = descriptorType;
	wdesc.pTexelBufferView = m_BufferViews.data() + m_NumBufferViews;
	m_DescriptorWrites.push_back(wdesc);
}

void ngv::VulkanDescriptorSetUpdater::bufferView(vk::BufferView view)
{
	if (!m_DescriptorWrites.empty() && m_NumBufferViews != m_BufferViews.size() && m_DescriptorWrites.back().pImageInfo) {
		m_DescriptorWrites.back().descriptorCount++;
		m_BufferViews[m_NumBufferViews++] = view;
	}
	else {
		m_Ok = false;
	}
}

void ngv::VulkanDescriptorSetUpdater::copy(vk::DescriptorSet srcSet, uint32_t srcBinding, uint32_t srcArrayElement, vk::DescriptorSet dstSet, uint32_t dstBinding, uint32_t dstArrayElement, uint32_t descriptorCount)
{
	m_DescriptorCopies.emplace_back(srcSet, srcBinding, srcArrayElement, dstSet, dstBinding, dstArrayElement, descriptorCount);
}

void ngv::VulkanDescriptorSetUpdater::update(const vk::Device& device) const
{
	device.updateDescriptorSets(m_DescriptorWrites, m_DescriptorCopies);
}

bool ngv::VulkanDescriptorSetUpdater::ok() const
{
	return m_Ok;
}
