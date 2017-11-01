#include "vulkan_allocator.h"


uint32 ng::graphics::VulkanAllocator::findMemoryType(VkPhysicalDevice* pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(*pDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

void ng::graphics::VulkanAllocator::init(VulkanBase * vkBase)
{
	m_VulkanBase = vkBase;

	for (int i = 0; i < m_VulkanBase->graphicsUnit.pDevice.memoryProperties.memoryHeapCount; ++i) {
		if (m_VulkanBase->graphicsUnit.pDevice.memoryProperties.memoryHeaps[i].flags == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			m_GraphicsMemorySize = (VkDeviceSize)(m_VulkanBase->graphicsUnit.pDevice.memoryProperties.memoryHeaps[i].size * 0.9);
		}
	}

	createBufferAndMemory(
		GRAPHICS_UNIT,
		&m_GraphicsBuffer,
		&m_GraphicsDeviceMemory,
		m_GraphicsMemorySize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SHARING_MODE_EXCLUSIVE
	);
	
	createBufferAndMemory(
		GRAPHICS_UNIT,
		&m_HostBuffer,
		&m_HostLocalMemory,
		m_HostLocalMemorySize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_SHARING_MODE_CONCURRENT
	);
	
	createBufferAndMemory(
		COMPUTE_UNIT,
		&m_ComputeBuffer,
		&m_ComputeDeviceMemory,
		m_ComputeDeviceMemorySize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SHARING_MODE_EXCLUSIVE
	);

}

void ng::graphics::VulkanAllocator::createBufferAndMemory(DeviceType deviceType, VkBuffer* buffer, VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = sharingMode;

	VkDevice* device;
	VkPhysicalDevice* pDevice;
	if (deviceType == GRAPHICS_UNIT) {
		device = &m_VulkanBase->graphicsUnit.device;
		pDevice = &m_VulkanBase->graphicsUnit.pDevice.device;
	}
	else if (deviceType == COMPUTE_UNIT){
		device = &m_VulkanBase->computeUnit.device;
		pDevice = &m_VulkanBase->computeUnit.pDevice.device;
	}
	else {
		throw std::runtime_error("no correct unit selected!");
	}

	if (vkCreateBuffer(*device, &createInfo, nullptr, buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(*device, *buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(pDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(*device, &allocInfo, nullptr, memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory");
	}
	vkBindBufferMemory(*device, *buffer, *memory, 0);
}
