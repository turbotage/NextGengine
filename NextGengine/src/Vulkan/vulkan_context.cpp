#include "vulkan_context.h"

#include "vulkan_instance.h"
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_utility.h"

ngv::VulkanContext::VulkanContext()
{
}

ngv::VulkanContext::VulkanContext(const std::string& name)
{
	ngv::VulkanInstanceMaker im{};
	im.setDefaultLayers();
	m_Instance = im.createUnique();

	m_DebugCallback = debug::VulkanDebugCallback(*m_Instance);

	auto pds = m_Instance->enumeratePhysicalDevices();
	m_PhysicalDevice = pds[0];
	auto qprops = m_PhysicalDevice.getQueueFamilyProperties();
	const auto badQueue = ~(uint32)0;
	m_GraphicsQueueFamilyIndex = badQueue;
	m_ComputeQueueFamilyIndex = badQueue;
	vk::QueueFlags search = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;

	for (uint32 qi = 0; qi != qprops.size(); ++qi) {
		auto& qprop = qprops[qi];
		std::cout << vk::to_string(qprop.queueFlags) << "\n";
		if ((qprop.queueFlags & search) == search) {
			m_GraphicsQueueFamilyIndex = qi;
			m_ComputeQueueFamilyIndex = qi;
			break;
		}
	}

	if (m_GraphicsQueueFamilyIndex == badQueue || m_ComputeQueueFamilyIndex == badQueue) {
		std::cout << "missing a queue\n";
		return;
	}

	m_MemProps = m_PhysicalDevice.getMemoryProperties();

	ngv::VulkanDeviceMaker dm{};
	dm.setDefaultLayers();
	dm.addQueue(m_GraphicsQueueFamilyIndex);
	if (m_ComputeQueueFamilyIndex != m_GraphicsQueueFamilyIndex) dm.addQueue(m_ComputeQueueFamilyIndex);
	m_Device = dm.createUnique(m_PhysicalDevice);

	vk::PipelineCacheCreateInfo pipelineCacheInfo{};
	m_PipelineCache = m_Device->createPipelineCacheUnique(pipelineCacheInfo);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, 128);
	poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, 128);
	poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, 128);

	vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	descriptorPoolInfo.maxSets = 256;
	descriptorPoolInfo.poolSizeCount = (uint32)poolSizes.size();
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	for (int i = 0; i < m_DescriptorPools.size(); ++i) {
		m_DescriptorPools[i] = m_Device->createDescriptorPoolUnique(descriptorPoolInfo);
	}

	m_Ok = true;
}

void ngv::VulkanContext::dumpCaps(std::ostream& os) const
{
	os << "Memory Types\n";
	for (uint32 i = 0; i != m_MemProps.memoryTypeCount; ++i) {
		os << "  type" << i << " heap" << m_MemProps.memoryTypes[i].heapIndex << " " << vk::to_string(m_MemProps.memoryTypes[i].propertyFlags);
	}
	os << "Heaps\n";
	for (uint32 i = 0; i != m_MemProps.memoryHeapCount; ++i) {
		os << "  heap" << vk::to_string(m_MemProps.memoryHeaps[i].flags) << " " << m_MemProps.memoryHeaps[i].size << "\n";
	}
}

const vk::Instance ngv::VulkanContext::instance() const
{
	return *m_Instance;
}

const vk::Device ngv::VulkanContext::device() const
{
	return *m_Device;
}

const vk::Queue ngv::VulkanContext::graphicsQueue() const
{
	return m_Device->getQueue(m_GraphicsQueueFamilyIndex, 0);
}

const vk::Queue ngv::VulkanContext::computeQueue() const
{
	return vk::Queue();
}

const vk::PhysicalDevice& ngv::VulkanContext::physicalDevice() const
{
	return m_PhysicalDevice;
}

const vk::PipelineCache ngv::VulkanContext::pipelineCache() const
{
	return *m_PipelineCache;
}

uint32_t ngv::VulkanContext::graphicsQueueFamilyIndex() const
{
	return m_GraphicsQueueFamilyIndex;
}

uint32 ngv::VulkanContext::computeQueueFamilyIndex() const
{
	return m_ComputeQueueFamilyIndex;
}

const vk::PhysicalDeviceMemoryProperties& ngv::VulkanContext::memProps() const
{
	return m_MemProps;
}



ngv::VulkanContext::~VulkanContext()
{
	if (m_Device) {
		m_Device->waitIdle();
		if (m_PipelineCache) {
			m_PipelineCache.reset();
		}
		for (auto& dPool : m_DescriptorPools) {
			if (dPool) {
				dPool.reset();
			}
		}
	}

	if (m_Instance) {
		m_DebugCallback.reset();
		m_Instance.reset();
	}
}

bool ngv::VulkanContext::ok() const
{
	return m_Ok;
}
