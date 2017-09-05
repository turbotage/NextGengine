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
	uint64_t size;



	createBufferAndMemory(
		GRAPHICS_UNIT,
		&m_GraphicsBuffer,
		&m_GraphicsDeviceMemory,
		size,
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
	else {
		device = &m_VulkanBase->computeUnit.device;
		pDevice = &m_VulkanBase->computeUnit.pDevice.device;
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
