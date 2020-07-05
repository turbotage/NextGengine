#include "vulkan_context.h"

#include <GLFW/glfw3.h>

#include "vulkan_instance.h"
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderpass.h"
#include "vulkan_utility.h"

ngv::VulkanContext::VulkanContext()
{
}

ngv::VulkanContext::VulkanContext(const std::string& name, uint32 version)
{
	m_pDevice = std::make_unique<VulkanDevice>();

	ngv::VulkanInstanceMaker im{};
	im.setDefaultLayers();

	uint32 nWindowExtensions;
	auto windowExtension = glfwGetRequiredInstanceExtensions(&nWindowExtensions);
	for (int i = 0; i < nWindowExtensions; ++i) {
		im.addExtension(windowExtension[i]);
	}

	im.setApiVersion(VK_API_VERSION_1_2);
	im.setApplicationName(name.c_str());
	im.setApplicationVersion(version);
	im.setEngineName(NG_ENGINE_NAME);
	im.setEngineVersion(NG_ENGINE_VERSION);

	m_Instance = im.createUnique();

	m_DebugCallback = debug::VulkanDebugCallback(*m_Instance);

	auto pds = m_Instance->enumeratePhysicalDevices();
	m_pDevice->setPhysicalDevice(pds[0]);
	const auto badQueue = ~(uint32)0;
	m_GraphicsQueueFamilyIndex = badQueue;
	m_ComputeQueueFamilyIndex = badQueue;
	vk::QueueFlags search = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;

	auto qprops = m_pDevice->queueFamilyProperties();
	for (uint32 qi = 0; qi != qprops.size(); ++qi) {
		auto& qprop = qprops[qi];
		//std::cout << vk::to_string(qprop.queueFlags) << "\n";
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

	ngv::VulkanDeviceMaker dm{};
	dm.setDefaultLayers();
	dm.addQueue(m_GraphicsQueueFamilyIndex);
	if (m_ComputeQueueFamilyIndex != m_GraphicsQueueFamilyIndex) dm.addQueue(m_ComputeQueueFamilyIndex);
	m_pDevice->setDevice(dm.createUnique(m_pDevice->physicalDevice()));

	vk::PipelineCacheCreateInfo pipelineCacheInfo{};
	m_PipelineCache = m_pDevice->device().createPipelineCacheUnique(pipelineCacheInfo);

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
		m_DescriptorPools[i] = m_pDevice->device().createDescriptorPoolUnique(descriptorPoolInfo);
	}

	m_Ok = true;
}

void ngv::VulkanContext::dumpCaps(std::ostream& os) const
{
	auto props = m_pDevice->physicalDeviceMemoryProperties();
	os << "Memory Types\n";
	for (uint32 i = 0; i != props.memoryTypeCount; ++i) {
		os << "  type" << i << " heap" << props.memoryTypes[i].heapIndex << " " << vk::to_string(props.memoryTypes[i].propertyFlags);
	}
	os << "Heaps\n";
	for (uint32 i = 0; i != props.memoryHeapCount; ++i) {
		os << "  heap" << vk::to_string(props.memoryHeaps[i].flags) << " " << props.memoryHeaps[i].size << "\n";
	}
}

const vk::Instance ngv::VulkanContext::instance() const
{
	return *m_Instance;
}

const vk::Device ngv::VulkanContext::device() const
{
	return m_pDevice->device();
}

const vk::Queue ngv::VulkanContext::graphicsQueue() const
{
	return m_pDevice->device().getQueue(m_GraphicsQueueFamilyIndex, 0);
}

const vk::Queue ngv::VulkanContext::computeQueue() const
{
	return m_pDevice->device().getQueue(m_ComputeQueueFamilyIndex, 0);
}

ngv::VulkanDevice& ngv::VulkanContext::vulkanDevice() const
{
	return *m_pDevice;
}

const vk::PhysicalDevice& ngv::VulkanContext::physicalDevice() const
{
	return m_pDevice->physicalDevice();
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
	return m_pDevice->physicalDeviceMemoryProperties();
}

ngv::VulkanContext::~VulkanContext()
{
	if (m_pDevice->device()) {
		m_pDevice->device().waitIdle();
		if (m_PipelineCache) {
			m_PipelineCache.reset();
		}
		for (auto& dPool : m_DescriptorPools) {
			if (dPool) {
				dPool.reset();
			}
		}
	}

	m_pDevice.reset();

	if (m_Instance) {
		m_DebugCallback.reset();
		m_Instance.reset();
	}
}

bool ngv::VulkanContext::ok() const
{
	return m_Ok;
}

