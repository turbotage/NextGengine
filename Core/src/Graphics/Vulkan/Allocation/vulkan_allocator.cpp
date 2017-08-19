#include "vulkan_allocator.h"



void ng::graphics::VulkanAllocator::init(VulkanBase * vkBase, AllocatorUsage allocUsage, DeviceType deviceType)
{
	m_AllocatorUsage = allocUsage;
	m_DeviceType = deviceType;
	vk::PhysicalDeviceMemoryProperties* memProps;
	vk::PhysicalDevice* pDevice;
	vk::MemoryPropertyFlags flags;

	switch (m_AllocatorUsage) {
	case eStaticStorageBuffer:
		flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
		break;
	case eDynamicStorageBuffer:
		flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		break;
	}

	switch (m_DeviceType) {
	case eGraphicsUnit:
		memProps = &m_VulkanBase->graphicsUnit.pDevice->memoryProperties;
		pDevice = &m_VulkanBase->graphicsUnit.pDevice->device;
		break;
	case eComputeUnit:
		memProps = &m_VulkanBase->computeUnit.pDevice->memoryProperties;
		pDevice = &m_VulkanBase->computeUnit.pDevice->device;
		break;
	}

	*memProps = pDevice->getMemoryProperties();
	uint8 heapTypeCount;
	heapTypeCount = memProps->memoryHeapCount;



	vk::MemoryAllocateInfo allocInfo;

}

void ng::graphics::VulkanAllocator::createBuffer(VkDeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	vk::BufferCreateInfo bufferInfo = {};

}
